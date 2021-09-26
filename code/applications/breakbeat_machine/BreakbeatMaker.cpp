/*
  ==============================================================================

    BreakbeatMaker.cpp
    Created: 4 Jul 2020 5:39:51pm
    Author:  Matthew

  ==============================================================================
*/

#include "BreakbeatMaker.h"

MainContentComponent::WaveformComponent::WaveformComponent(MainContentComponent& parent, juce::AudioFormatManager& formatManager)
: mParentComponent(parent)
, mAudioFormatManager(formatManager)
, mThumbnailCache(5)
, mThumbnail(512, mAudioFormatManager, mThumbnailCache)
{
}

MainContentComponent::WaveformComponent::~WaveformComponent()
{
    
}


juce::AudioThumbnail& MainContentComponent::WaveformComponent::getThumbnail()
{
    return mThumbnail;
}

void MainContentComponent::WaveformComponent::setSampleStartEnd(int64_t start, int64_t end)
{
    mStartSample = std::max(start, static_cast<int64_t>(0));
    mEndSample = std::min(end, static_cast<int64_t>(mThumbnail.getTotalLength() * mSampleRate));
    
    triggerAsyncUpdate();
}

void MainContentComponent::WaveformComponent::resized()
{
    
}

void MainContentComponent::WaveformComponent::paint(juce::Graphics& g)
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

bool MainContentComponent::WaveformComponent::isInterestedInFileDrag (const StringArray& files)
{
    for(auto fileName : files)
    {
        if(!fileName.endsWith(".wav") && !fileName.endsWith(".aif") && !fileName.endsWith(".aiff"))
            return false;
    }
    
    return true;
}

void MainContentComponent::WaveformComponent::filesDropped (const StringArray& files, int x, int y)
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

void MainContentComponent::WaveformComponent::handleAsyncUpdate()
{
    repaint();
}

MainContentComponent::MainContentComponent(juce::AudioDeviceManager& audioDeviceManager, juce::RecentlyOpenedFilesList& recentFiles)
: juce::AudioAppComponent(audioDeviceManager)
, juce::Thread("Background Thread")
, mSliceDivsorSlider("Slice Div", "")
, mChangeSampleProbabilitySlider("Swap slice", "%")
, mReverseSampleProbabilitySlider("Reverse slice", "%")
, mRecentFiles(recentFiles)
{
    addAndMakeVisible(mSampleBpmField);
    mSampleBpmField.onValueChanged = [this](double value)
    {
        mAudioSource.setSampleBpm(value);
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
    mChangeSampleProbabilitySlider.setValue(0.3, dontSendNotification);
    mChangeSampleProbabilitySlider.onValueChange = [this]()
    {
        mSampleChangeThreshold = 1.0f - static_cast<float>(mChangeSampleProbabilitySlider.getValue());
        mAudioSource.setSampleChangeThreshold(mSampleChangeThreshold);
    };
    
    addAndMakeVisible(mReverseSampleProbabilitySlider);
    mReverseSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mReverseSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mReverseSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mReverseSampleProbabilitySlider.setValue(0.3, dontSendNotification);
    mReverseSampleProbabilitySlider.onValueChange = [this]()
    {
        mReverseSampleThreshold = 1.0f - static_cast<float>(mReverseSampleProbabilitySlider.getValue());
        mAudioSource.setReverseSampleThreshold(mReverseSampleThreshold) ;
    };
    
    addAndMakeVisible(mWaveformComponent);
    
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
            mRecordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::recordingButtonColourId));
        }
        else
        {
            mRecorder.stopRecording();
            mRecordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::defaultButtonColourId));
        }
    };
    
    addAndMakeVisible(mFileNameLabel);
    mFileNameLabel.setEditable(false);
    
    addAndMakeVisible(mFileSampleRateLabel);
    mFileSampleRateLabel.setEditable(false);
    
    setSize (500, 540);

    mFormatManager.registerBasicFormats();
    
    mTransportSource.addChangeListener(this);
    
    mWaveformComponent.getThumbnail().addChangeListener(this);
    
    startThread();
}

MainContentComponent::~MainContentComponent()
{
    stopThread (4000);
    shutdownAudio();
}

void MainContentComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);
    
    auto const twoFieldRowElementWidth = bounds.getWidth() / 3;
    auto const twoFieldRowSpacing = bounds.getWidth() - twoFieldRowElementWidth * 2;
    
    auto const threeFieldRowElementWidth = bounds.getWidth() / 4;
    auto const threeFieldRowSpacing = static_cast<int>((bounds.getWidth() - threeFieldRowElementWidth * 3) / 2.0);
    
    auto secondRowBounds = bounds.removeFromTop(100);
    mSampleBpmField.setBounds(secondRowBounds.removeFromLeft(twoFieldRowElementWidth).removeFromTop(20));
    secondRowBounds.removeFromLeft(twoFieldRowSpacing);
    mSliceDivsorSlider.setBounds(secondRowBounds.removeFromLeft(twoFieldRowSpacing));
    
    bounds.removeFromTop(20);
    
    auto thirdRowBounds = bounds.removeFromTop(100);
    mChangeSampleProbabilitySlider.setBounds(thirdRowBounds.removeFromLeft(twoFieldRowElementWidth));
    thirdRowBounds.removeFromLeft(twoFieldRowSpacing);
    mReverseSampleProbabilitySlider.setBounds(thirdRowBounds.removeFromLeft(twoFieldRowElementWidth));
    
    bounds.removeFromTop(20);
    
    auto fourthRowBounds = bounds.removeFromTop(220);
    mWaveformComponent.setBounds(fourthRowBounds.removeFromTop(200));
    mFileNameLabel.setBounds(fourthRowBounds.removeFromLeft(twoFieldRowElementWidth));
    fourthRowBounds.removeFromLeft(twoFieldRowSpacing);
    mFileSampleRateLabel.setBounds(fourthRowBounds.removeFromLeft(twoFieldRowElementWidth));
    
    bounds.removeFromTop(20);
    
    auto fifthRowBounds = bounds.removeFromTop(20);
    mPlayButton.setBounds(fifthRowBounds.removeFromLeft(threeFieldRowElementWidth));
    fifthRowBounds.removeFromLeft(threeFieldRowSpacing);
    mRecordButton.setBounds(fifthRowBounds.removeFromLeft(threeFieldRowElementWidth));
    fifthRowBounds.removeFromLeft(threeFieldRowSpacing);
    mStopButton.setBounds(fifthRowBounds.removeFromLeft(threeFieldRowElementWidth));
}

void MainContentComponent::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void MainContentComponent::lookAndFeelChanged()
{
    getLookAndFeel().setColour (MainContentComponent::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    getLookAndFeel().setColour (MainContentComponent::ColourIds::playingButtonColourId, juce::Colours::green);
    getLookAndFeel().setColour(MainContentComponent::ColourIds::defaultButtonColourId, findColour(juce::TextButton::ColourIds::buttonColourId));
    getLookAndFeel().setColour(MainContentComponent::ColourIds::recordingButtonColourId, juce::Colours::red);
}

void MainContentComponent::prepareToPlay (int samplerPerBlockExpected, double sampleRate)
{
    mAudioSource.prepareToPlay(samplerPerBlockExpected, sampleRate);
    mTransportSource.prepareToPlay(samplerPerBlockExpected, sampleRate);
    
    mTemporaryChannels.resize(2, nullptr);
}

void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
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
    
    mWaveformComponent.setSampleStartEnd(mAudioSource.getStartReadPosition(), mAudioSource.getEndReadPosition());
}

void MainContentComponent::releaseResources()
{
    mTransportSource.releaseResources();
    mAudioSource.releaseResources();
}

void MainContentComponent::run()
{
    while(!threadShouldExit())
    {
        checkForPathToOpen();
        checkForBuffersToFree();
        wait(500);
    }
}

void MainContentComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
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

void MainContentComponent::handleAsyncUpdate()
{
    mFileNameLabel.setText(mFileName, juce::NotificationType::dontSendNotification);
    mFileSampleRateLabel.setText(juce::String(mFileSampleRate), juce::NotificationType::dontSendNotification);
    
    changeState(TransportState::Stopped);
    
    if(mFileSource != nullptr)
    {
        mWaveformComponent.getThumbnail().setSource(mFileSource.get());
        mFileSource.release();
    }
}

void MainContentComponent::newFileOpened(String& filePath)
{
    mChosenPath.swapWith(filePath);
    notify();
}

void MainContentComponent::setFileOutputPath()
{
    juce::FileChooser fileChooser ("Please select the location you'd like to record to...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
    
    if(fileChooser.browseForFileToSave(true))
    {
        mRecordedFile = fileChooser.getResult();
    }
}

void MainContentComponent::exportAudioSlices()
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
        auto* readBuffer = mAudioSource.getCurrentBuffer();
        auto fileName = file.getFileNameWithoutExtension();
        auto path = file.getParentDirectory().getFullPathName();
        mSliceExporter.startExport(readBuffer, fileName, path, mAudioSource.getSliceSize(), mAudioSource.getNumSlices(), 2, 44100.0, 32);
    }
    catch (std::exception e)
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to export slices", e.what());
    }
}

void MainContentComponent::changeState(TransportState state)
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
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::defaultButtonColourId));
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::defaultButtonColourId));
                mTransportSource.setPosition(0.0);
                mAudioSource.setNextReadPosition(0.0);
                
                if(mRecording)
                {
                    mRecorder.stopRecording();
                    mRecordButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::defaultButtonColourId));
                }
            }
                break;
                
            case TransportState::Starting:
                mPlayButton.setEnabled(false);
                mTransportSource.start();
                break;
                
            case TransportState::Playing:
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::playingButtonColourId));
                mPlayButton.setColour(juce::TextButton::ColourIds::buttonColourId, getLookAndFeel().findColour(MainContentComponent::ColourIds::playingButtonColourId));
                mStopButton.setEnabled(true);
                break;
                
            case TransportState::Stopping:
                mTransportSource.stop();
        }
    }
}

void MainContentComponent::checkForPathToOpen()
{
    juce::String pathToOpen;
    pathToOpen.swapWith(mChosenPath);
    
    if(pathToOpen.isEmpty())
    {
        return;
    }
    
    juce::File file(pathToOpen);
    std::unique_ptr<AudioFormatReader> reader { mFormatManager.createReaderFor(file) };
    
    if(reader != nullptr)
    {
        // get length
        auto duration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
        if(duration < MAX_FILE_LENGTH)
        {
            mTransportSource.setSource(&mAudioSource, 0, nullptr, reader->sampleRate);
            mAudioSource.setReader(reader.get());
            mFileSource = std::make_unique<juce::FileInputSource>(file);
            
            mFileName = file.getFileName();
            mFileSampleRate = reader->sampleRate;
            mRecentFiles.addFile(file);
            
            triggerAsyncUpdate();
        }
        else
        {
             juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                                          juce::translate("Audio file too long!"),
                                               juce::translate("The file is ") + juce::String(duration) + juce::translate("seconds long. ") + juce::String(MAX_FILE_LENGTH) + "Second limit!");
        }
        
    }
}

void MainContentComponent::checkForBuffersToFree()
{
    mAudioSource.clearFreeBuffers();
}
