/*
  ==============================================================================

    BreakbeatMaker.cpp
    Created: 4 Jul 2020 5:39:51pm
    Author:  Matthew

  ==============================================================================
*/

#include "BreakbeatMaker.h"

BreakbeatContentComponent::WaveformComponent::WaveformComponent(BreakbeatContentComponent& parent, juce::AudioFormatManager& formatManager)
: mParentComponent(parent)
, mAudioFormatManager(formatManager)
, mThumbnailCache(1)
, mThumbnail(512, mAudioFormatManager, mThumbnailCache)
{
}

BreakbeatContentComponent::WaveformComponent::~WaveformComponent()
{
    
}


juce::AudioThumbnail& BreakbeatContentComponent::WaveformComponent::getThumbnail()
{
    return mThumbnail;
}

void BreakbeatContentComponent::WaveformComponent::clear()
{
    mThumbnailCache.clear();
    mThumbnail.clear();
}

void BreakbeatContentComponent::WaveformComponent::setSampleStartEnd(int64_t start, int64_t end)
{
    mStartSample = std::max(start, static_cast<int64_t>(0));
    mEndSample = std::min(end, static_cast<int64_t>(mThumbnail.getTotalLength() * mSampleRate));
    
    triggerAsyncUpdate();
}

void BreakbeatContentComponent::WaveformComponent::resized()
{
    
}

void BreakbeatContentComponent::WaveformComponent::paint(juce::Graphics& g)
{
    juce::Rectangle<int> thumbnailBounds (10, 10, getWidth()-20, getHeight()-20);
    
    if(mThumbnail.getNumChannels() == 0)
    {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(thumbnailBounds);
        g.setColour(juce::Colours::white);
        g.drawFittedText("Drag and drop and audio file", thumbnailBounds, juce::Justification::centred, 1);
    }
    else
    {
        g.setColour(juce::Colours::white);
        g.fillRect(thumbnailBounds);
        g.setColour(juce::Colours::red);
        mThumbnail.drawChannels(g, thumbnailBounds, 0.0, mThumbnail.getTotalLength(), 1.0f);
    }
    
    juce::Range<int64_t> sampleRange { mStartSample, mEndSample };
    if(sampleRange.getLength() == 0)
    {
        return;
    }
    
    auto const sampleStartRatio = static_cast<double>(sampleRange.getStart() / mSampleRate) / mThumbnail.getTotalLength();
    auto const sampleSizeRatio = static_cast<double>(sampleRange.getLength() / mSampleRate) / mThumbnail.getTotalLength();
    
    juce::Rectangle<int> clipBounds {
        thumbnailBounds.getX() + static_cast<int>(thumbnailBounds.getWidth() * sampleStartRatio),
        thumbnailBounds.getY(),
        static_cast<int>(thumbnailBounds.getWidth() * sampleSizeRatio),
        thumbnailBounds.getHeight()
    };
    
    g.setColour(juce::Colours::blue.withAlpha(0.4f));
    g.fillRect(clipBounds);
}

bool BreakbeatContentComponent::WaveformComponent::isInterestedInFileDrag (const StringArray& files)
{
    for(auto fileName : files)
    {
        if(!fileName.endsWith(".wav") && !fileName.endsWith(".aif") && !fileName.endsWith(".aiff"))
            return false;
    }
    
    return true;
}

void BreakbeatContentComponent::WaveformComponent::filesDropped (const StringArray& files, int x, int y)
{
    // only deal with one file for now.
    juce::ignoreUnused(x, y);
    
    auto const filePath = files[0];
    juce::File f { filePath };
    
    if(f.existsAsFile())
    {
        auto path = f.getFullPathName();
        mParentComponent.newFileOpened(path);
    }
}

void BreakbeatContentComponent::WaveformComponent::handleAsyncUpdate()
{
    repaint();
}

BreakbeatContentComponent::BreakbeatContentComponent(juce::AudioDeviceManager& audioDeviceManager, juce::RecentlyOpenedFilesList& recentFiles)
: juce::AudioAppComponent(audioDeviceManager)
, juce::Thread("Background Thread")
, mPitchShiftSlider("Pitch shift", "")
, mCrossFadeSlider("Cross fade", "ms")
, mSliceDivsorSlider("Slice Div", "")
, mChangeSampleProbabilitySlider("Swap slice", "%")
, mReverseSampleProbabilitySlider("Reverse slice", "%")
, mRetriggerSampleProbabilitySlider("Retrigger slice", "%")
, mRecentFiles(recentFiles)
{
    addAndMakeVisible(mPitchShiftSlider);
    mPitchShiftSlider.mLabels.add({0.0, "-24"});
    mPitchShiftSlider.mLabels.add({1.0, "24"});
    mPitchShiftSlider.setRange(-24.0, 24.0, 0.1);
    mPitchShiftSlider.setValue(0.0, juce::NotificationType::dontSendNotification);
    mPitchShiftSlider.onValueChange = [this]()
    {
        auto const stretchFactor = static_cast<float>(mAudioSource.getSliceManager().getBufferLength() / mAudioSource.getSliceManager().getFileLength());
        auto const pitchFactor = static_cast<float>(std::pow(2.0, mPitchShiftSlider.getValue() / 12.0));
        
        mAudioSource.getSliceManager().performTimestretch(stretchFactor, pitchFactor, [this]()
        {
            mPlayButton.setEnabled(true);
            mAudioSource.getSliceManager().performSlice();
            mAudioSource.setNextReadPosition(0);
            updateWaveform();
        });
    };
    
    addAndMakeVisible(mCrossFadeSlider);
    mCrossFadeSlider.mLabels.add({0.0, "0"});
    mCrossFadeSlider.mLabels.add({1.0, "200"});
    mCrossFadeSlider.setRange(0.0, 400.0, 10.0);
    mCrossFadeSlider.setValue(100, juce::NotificationType::dontSendNotification);
    mCrossFadeSlider.onValueChange = [this]()
    {
        mAudioSource.setCrossFade(static_cast<float>(mCrossFadeSlider.getValue()));
    };
    
    addAndMakeVisible(mSliceDivsorSlider);
    mSliceDivsorSlider.mLabels.add({0.0, "0"});
    mSliceDivsorSlider.mLabels.add({1.0, "256"});
    mSliceDivsorSlider.setRange(0.0, 8.0, 1.0);
    mSliceDivsorSlider.setValue(1.0, dontSendNotification);
    mSliceDivsorSlider.onValueChange = [this]()
    {
        auto const divisor = static_cast<int>(std::pow(2, static_cast<int>(mSliceDivsorSlider.getValue())));
        mAudioSource.setBlockDivisionFactor(divisor);
    };
    
    addAndMakeVisible(mChangeSampleProbabilitySlider);
    mChangeSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mChangeSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mChangeSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mChangeSampleProbabilitySlider.setValue(0.3, juce::NotificationType::dontSendNotification);
    mChangeSampleProbabilitySlider.onValueChange = [this]()
    {
        mSampleChangeThreshold = 1.0f - static_cast<float>(mChangeSampleProbabilitySlider.getValue());
        mAudioSource.setSampleChangeThreshold(mSampleChangeThreshold);
    };
    
    addAndMakeVisible(mReverseSampleProbabilitySlider);
    mReverseSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mReverseSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mReverseSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mReverseSampleProbabilitySlider.setValue(0.3, juce::NotificationType::dontSendNotification);
    mReverseSampleProbabilitySlider.onValueChange = [this]()
    {
        mReverseSampleThreshold = 1.0f - static_cast<float>(mReverseSampleProbabilitySlider.getValue());
        mAudioSource.setReverseSampleThreshold(mReverseSampleThreshold) ;
    };
    
    addAndMakeVisible(mRetriggerSampleProbabilitySlider);
    mRetriggerSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mRetriggerSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mRetriggerSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mRetriggerSampleProbabilitySlider.setValue(0.3, juce::NotificationType::dontSendNotification);
    mRetriggerSampleProbabilitySlider.onValueChange = [this]()
    {
        mRetriggerSampleThreshold = 1.0f - static_cast<float>(mRetriggerSampleProbabilitySlider.getValue());
        mAudioSource.setRetriggerSampleThreshold(mRetriggerSampleThreshold);
    };
    
    addAndMakeVisible(mWaveformComponent);
    
    addAndMakeVisible(mSampleLengthSeconds);
    mSampleDesiredLengthSeconds.setNumberOfDecimals(3);
    
    addAndMakeVisible(mSampleDesiredLengthSeconds);
    mSampleDesiredLengthSeconds.setNumberOfDecimals(3);
    mSampleDesiredLengthSeconds.setRange({0.1, 10.0}, juce::NotificationType::dontSendNotification);
    mSampleDesiredLengthSeconds.onValueChanged = [this](double value)
    {
        juce::ignoreUnused(value);
        
        changeState(TransportState::Stopping);
        mPlayButton.setEnabled(false);
        
        auto const stretchFactor = static_cast<float>(value / mAudioSource.getSliceManager().getFileLength());
        auto const pitchFactor = static_cast<float>(std::pow(2.0, mPitchShiftSlider.getValue() / 12.0));
        mAudioSource.getSliceManager().performTimestretch(stretchFactor, pitchFactor, [this]()
        {
            mPlayButton.setEnabled(true);
            mAudioSource.getSliceManager().performSlice();
            mAudioSource.setNextReadPosition(0);
            updateWaveform();
        });
    };
    
    addAndMakeVisible(mStopButton);
    mStopButton.setButtonText("Stop");
    mStopButton.onClick = [this]()
    {
        changeState(TransportState::Stopping);
    };
    
    addAndMakeVisible(mPlayButton);
    mPlayButton.setButtonText("Play");
    mPlayButton.onClick = [this]()
    {
        changeState(TransportState::Starting);
    };
    
    addAndMakeVisible(mRecordButton);
    mRecordButton.setButtonText("Record");
    mRecordButton.onClick = [this]()
    {
        mRecording = !mRecording;
        if(mRecording)
        {
            mRecorder.startRecording(mRecordedFile, 2, 44100.0, 32);
            mRecordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::recordingButtonColourId));
        }
        else
        {
            mRecorder.stopRecording();
            mRecordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::defaultButtonColourId));
        }
    };
    
    addAndMakeVisible(mFileNameLabel);
    mFileNameLabel.setEditable(false);
    
    addAndMakeVisible(mFileSampleRateLabel);
    mFileSampleRateLabel.setEditable(false);
    
    setSize (500, 580);

    mFormatManager.registerBasicFormats();
    
    mTransportSource.addChangeListener(this);
    
    mWaveformComponent.getThumbnail().addChangeListener(this);
    
    startThread();
}

BreakbeatContentComponent::~BreakbeatContentComponent()
{
    stopThread (4000);
    shutdownAudio();
}

void BreakbeatContentComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);
    
    auto const twoFieldRowElementWidth = bounds.getWidth() / 3;
    auto const twoFieldRowSpacing = bounds.getWidth() - twoFieldRowElementWidth * 2;
    
    auto const threeFieldRowElementWidth = bounds.getWidth() / 4;
    auto const threeFieldRowSpacing = static_cast<int>((bounds.getWidth() - threeFieldRowElementWidth * 3) / 2.0);
    
    auto secondRowBounds = bounds.removeFromTop(100);
    mPitchShiftSlider.setBounds(secondRowBounds.removeFromLeft(threeFieldRowElementWidth));
    secondRowBounds.removeFromLeft(threeFieldRowSpacing);
    mCrossFadeSlider.setBounds(secondRowBounds.removeFromLeft(threeFieldRowElementWidth));
    secondRowBounds.removeFromLeft(threeFieldRowSpacing);
    mSliceDivsorSlider.setBounds(secondRowBounds.removeFromLeft(threeFieldRowElementWidth));
    
    bounds.removeFromTop(20);
    
    auto thirdRowBounds = bounds.removeFromTop(100);
    mChangeSampleProbabilitySlider.setBounds(thirdRowBounds.removeFromLeft(threeFieldRowElementWidth));
    thirdRowBounds.removeFromLeft(threeFieldRowSpacing);
    mRetriggerSampleProbabilitySlider.setBounds(thirdRowBounds.removeFromLeft(threeFieldRowElementWidth));
    thirdRowBounds.removeFromLeft(threeFieldRowSpacing);
    mReverseSampleProbabilitySlider.setBounds(thirdRowBounds.removeFromLeft(threeFieldRowElementWidth));
    
    bounds.removeFromTop(20);
    
    auto fourthRowBounds = bounds.removeFromTop(220);
    mWaveformComponent.setBounds(fourthRowBounds.removeFromTop(200));
    mFileNameLabel.setBounds(fourthRowBounds.removeFromLeft(twoFieldRowElementWidth));
    fourthRowBounds.removeFromLeft(twoFieldRowSpacing);
    mFileSampleRateLabel.setBounds(fourthRowBounds.removeFromLeft(twoFieldRowElementWidth));
    
    bounds.removeFromTop(20);
    
    auto fifthRowBounds = bounds.removeFromTop(20);
    mSampleLengthSeconds.setBounds(fifthRowBounds.removeFromLeft(twoFieldRowElementWidth));
    fifthRowBounds.removeFromLeft(twoFieldRowSpacing);
    mSampleDesiredLengthSeconds.setBounds(fifthRowBounds.removeFromLeft(twoFieldRowElementWidth));
    
    bounds.removeFromTop(20);
    
    auto sixthRowBounds = bounds.removeFromTop(20);
    mPlayButton.setBounds(sixthRowBounds.removeFromLeft(threeFieldRowElementWidth));
    sixthRowBounds.removeFromLeft(threeFieldRowSpacing);
    mRecordButton.setBounds(sixthRowBounds.removeFromLeft(threeFieldRowElementWidth));
    sixthRowBounds.removeFromLeft(threeFieldRowSpacing);
    mStopButton.setBounds(sixthRowBounds.removeFromLeft(threeFieldRowElementWidth));
}

void BreakbeatContentComponent::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void BreakbeatContentComponent::lookAndFeelChanged()
{
    getLookAndFeel().setColour (BreakbeatContentComponent::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    getLookAndFeel().setColour (BreakbeatContentComponent::ColourIds::playingButtonColourId, juce::Colours::green);
    getLookAndFeel().setColour(BreakbeatContentComponent::ColourIds::defaultButtonColourId, findColour(juce::TextButton::ColourIds::buttonColourId));
    getLookAndFeel().setColour(BreakbeatContentComponent::ColourIds::recordingButtonColourId, juce::Colours::red);
}

void BreakbeatContentComponent::prepareToPlay (int samplerPerBlockExpected, double sampleRate)
{
    mAudioSource.prepareToPlay(samplerPerBlockExpected, sampleRate);
    mTransportSource.prepareToPlay(samplerPerBlockExpected, sampleRate);
    
    mTemporaryChannels.resize(2, nullptr);
}

void BreakbeatContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    mTransportSource.getNextAudioBlock(bufferToFill);
    
    auto& buffer = *(bufferToFill.buffer);
    auto const numChannels = std::min(static_cast<int>(mTemporaryChannels.size()), buffer.getNumChannels());
    auto const numSamples = bufferToFill.numSamples;
    auto const startSample = bufferToFill.startSample;
    for(int ch = 0; ch < numChannels; ++ch)
    {
        mTemporaryChannels[static_cast<size_t>(ch)] = buffer.getWritePointer(ch, startSample);
    }
    
    juce::AudioBuffer<float> localBuffer(mTemporaryChannels.data(), numChannels, numSamples);
    mRecorder.processBlock(localBuffer);
    
    auto const currentSlice = mAudioSource.getSliceManager().getCurrentSlice();
    auto const start = std::get<0>(currentSlice);
    auto const end = std::get<1>(currentSlice);
    
    if(start > end || end - start == 0)
    {
        return;
    }
    
    mWaveformComponent.setSampleStartEnd(start, end);
}

void BreakbeatContentComponent::releaseResources()
{
    mTransportSource.releaseResources();
    mAudioSource.releaseResources();
}

void BreakbeatContentComponent::run()
{
    while(!threadShouldExit())
    {
        checkForPathToOpen();
        checkForBuffersToFree();
        wait(500);
    }
}

void BreakbeatContentComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if(source == &mWaveformComponent.getThumbnail())
    {
        repaint();
    }
    else if(source == &mTransportSource)
    {
        changeState(mTransportSource.isPlaying() ? TransportState::Playing : TransportState::Stopped);
    }
}

void BreakbeatContentComponent::handleAsyncUpdate()
{
    mFileNameLabel.setText(mAudioSource.getSliceManager().getSampleFileName(), juce::NotificationType::dontSendNotification);
    mFileSampleRateLabel.setText(juce::String(mAudioSource.getSliceManager().getSampleSampleRate()), juce::NotificationType::dontSendNotification);
    
    mSampleLengthSeconds.setValue(mAudioSource.getSliceManager().getFileLength(), juce::NotificationType::sendNotification);
    
    auto const pitchFactor = static_cast<float>(std::pow(2.0, mPitchShiftSlider.getValue() / 12.0));
    mAudioSource.getSliceManager().performTimestretch(1.0f, pitchFactor, [this]()
    {
        mSampleDesiredLengthSeconds.setValue(mAudioSource.getSliceManager().getBufferLength(), juce::NotificationType::dontSendNotification);
        mPlayButton.setEnabled(true);
        mAudioSource.getSliceManager().performSlice();
        mAudioSource.setNextReadPosition(0);
        
        updateWaveform();
    });
    
    changeState(TransportState::Stopped);
}

void BreakbeatContentComponent::newFileOpened(juce::String& filePath)
{
    mChosenPath.swapWith(filePath);
    notify();
}

void BreakbeatContentComponent::setFileOutputPath()
{
    juce::FileChooser fileChooser ("Please select the location you'd like to record to...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
    
    if(fileChooser.browseForFileToSave(true))
    {
        mRecordedFile = fileChooser.getResult();
    }
}

void BreakbeatContentComponent::exportAudioSlices()
{
    // TODO: Check its not already exporting; cleanup
    juce::FileChooser fileChooser("Export slices to file(s)...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
    if(!fileChooser.browseForFileToSave(true))
    {
        return;
    }
    
    auto file = fileChooser.getResult();
    try
    {
//        auto* readBuffer = mAudioSource.getCurrentBuffer();
//        auto fileName = file.getFileNameWithoutExtension();
//        auto path = file.getParentDirectory().getFullPathName();
//        mSliceExporter.startExport(readBuffer, fileName, path, mAudioSource.getSliceSize(), mAudioSource.getNumSlices(), 2, 44100.0, 32);
    }
    catch (std::exception e)
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to export slices", e.what());
    }
}

void BreakbeatContentComponent::changeState(TransportState state)
{
    if(mState != state)
    {
        mState = state;
        
        switch(state)
        {
            case TransportState::Stopped:
            {
                mStopButton.setEnabled(false);
                mPlayButton.setEnabled(true);
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::defaultButtonColourId));
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::defaultButtonColourId));
                mTransportSource.setPosition(0.0);
                mAudioSource.setNextReadPosition(0.0);
                
                if(mRecording)
                {
                    mRecorder.stopRecording();
                    mRecordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::defaultButtonColourId));
                }
            }
                break;
                
            case TransportState::Starting:
                mPlayButton.setEnabled(false);
                mTransportSource.start();
                break;
                
            case TransportState::Playing:
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::playingButtonColourId));
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(BreakbeatContentComponent::ColourIds::playingButtonColourId));
                mStopButton.setEnabled(true);
                break;
                
            case TransportState::Stopping:
                mTransportSource.stop();
        }
    }
}

void BreakbeatContentComponent::checkForPathToOpen()
{
    juce::String pathToOpen;
    pathToOpen.swapWith(mChosenPath);
    
    if(pathToOpen.isEmpty())
    {
        return;
    }
    
    juce::String error;
    if(mAudioSource.getSliceManager().loadNewSample(pathToOpen, error))
    {
        mTransportSource.setSource(&mAudioSource, 0, nullptr, mAudioSource.getSliceManager().getSampleSampleRate());
    }
    else
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                          juce::translate("Load sample failed!"), error);
        return;
    }
    
    triggerAsyncUpdate();
}

void BreakbeatContentComponent::checkForBuffersToFree()
{
    mAudioSource.getSliceManager().clearFreeBuffers();
}

void BreakbeatContentComponent::updateWaveform()
{
    mWaveformComponent.clear();
    mWaveformComponent.getThumbnail().reset(2, mAudioSource.getSliceManager().getSampleSampleRate());
    mWaveformComponent.getThumbnail().addBlock(0, *mAudioSource.getSliceManager().getActiveBuffer(), 0, static_cast<int>(mAudioSource.getSliceManager().getBufferNumSamples()));
}
