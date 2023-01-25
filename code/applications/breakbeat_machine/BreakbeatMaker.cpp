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

void BreakbeatContentComponent::WaveformComponent::setSlicePositions(std::vector<SliceManager::Slice> const& slices, size_t activeSliceIndex)
{
    mSlicePositions.clear();
    mSlicePositions.resize(slices.size());
    
    for(size_t i = 0; i < mSlicePositions.size(); ++i)
    {
        mSlicePositions[i] = std::get<1>(slices[i]);
    }
    
    mActiveSliceIndex = activeSliceIndex;
    
    repaint();
}

void BreakbeatContentComponent::WaveformComponent::setActiveSlice(size_t sliceIndex)
{
    mActiveSliceIndex = sliceIndex;
    repaint();
}

void BreakbeatContentComponent::WaveformComponent::resized()
{
    
}

void BreakbeatContentComponent::WaveformComponent::paint(juce::Graphics& g)
{
    juce::Rectangle<int> thumbnailBounds (0, 20, getWidth(), getHeight());
    
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
    
    if(mSlicePositions.size() <= 1)
    {
        return;
    }
    
    // paint all of the slices
    g.setColour(juce::Colours::black);
    for(size_t i = 0; i < mSlicePositions.size(); ++i)
    {
        auto const startRatio = static_cast<double>(mSlicePositions[i] / mSampleRate) / mThumbnail.getTotalLength();
        auto const sliceX = thumbnailBounds.getX() + static_cast<int>(thumbnailBounds.getWidth() * startRatio);
        g.drawVerticalLine(sliceX, thumbnailBounds.getY(), thumbnailBounds.getBottom());
        
        // draw the slice handle
        Path trianglePath;
        trianglePath.addTriangle(sliceX - 8.0f, thumbnailBounds.getY(), sliceX, thumbnailBounds.getY() - 16.0f, sliceX + 8.0f, thumbnailBounds.getY());
        g.fillPath(trianglePath);
    }
    
    // paint active slice range;
    auto const activeSliceStart = mSlicePositions[mActiveSliceIndex];
    auto const activeSliceEnd = mActiveSliceIndex >= mSlicePositions.size() ? static_cast<size_t>(mThumbnail.getTotalLength() * mSampleRate) : mSlicePositions[mActiveSliceIndex + 1];
    
    juce::Range<size_t> sampleRange { activeSliceStart, activeSliceEnd };
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

void BreakbeatContentComponent::WaveformComponent::mouseDoubleClick(juce::MouseEvent const& event)
{
    if(onWaveformDoubleClicked != nullptr)
    {
        onWaveformDoubleClicked(event.x);
    }
}

void BreakbeatContentComponent::WaveformComponent::mouseDown(juce::MouseEvent const& event)
{
    if(event.y > 20)
    {
        return;
    }
    
    // TODO: Remove duplication with mouseUp method
    for(size_t i = 0; i < mSlicePositions.size(); ++i)
    {
        auto const startRatio = static_cast<double>(mSlicePositions[i] / mSampleRate) / mThumbnail.getTotalLength();
        auto const sliceX = static_cast<int>(getWidth() * startRatio);
        
        if(event.x > sliceX - 8 && event.x < sliceX + 8)
        {
            if(onSliceMarkerMouseDown != nullptr)
            {
                onSliceMarkerMouseDown(sliceX);
            }
            
            break;
        }
    }
}

void BreakbeatContentComponent::WaveformComponent::mouseUp(juce::MouseEvent const& event)
{
    if(event.mods.isRightButtonDown())
    {
        if(event.y > 20)
        {
            // we know its not on a marker triangle
            return;
        }
        
        // check all slices to check if its in the triangle bounds
        for(size_t i = 0; i < mSlicePositions.size(); ++i)
        {
            auto const startRatio = static_cast<double>(mSlicePositions[i] / mSampleRate) / mThumbnail.getTotalLength();
            auto const sliceX = static_cast<int>(getWidth() * startRatio);
            
            if(event.x > sliceX - 8 && event.x < sliceX + 8)
            {
                if(onSliceMarkerRightClicked != nullptr)
                {
                    onSliceMarkerRightClicked(sliceX);
                }
                
                break;
            }
        }
    }
    
    if(onMouseUp != nullptr)
    {
        onMouseUp();
    }
}

void BreakbeatContentComponent::WaveformComponent::mouseDrag(const MouseEvent& event)
{
    // increment position of the marker
    if(onSliceMarkerDragged != nullptr)
    {
        onSliceMarkerDragged(event.position.x);
    }
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
, mCrossFadeSlider("Cross fade", "ms", 100.0)
, mSliceDivsorSlider("Slice Div", "", 1.0)
, mSliceTransientThresholdSlider("Detection Thresh.", "", 0.3)
, mChangeSampleProbabilitySlider("Swap slice", "%", 0.3)
, mReverseSampleProbabilitySlider("Reverse slice", "%", 0.3)
, mRetriggerSampleProbabilitySlider("Retrigger slice", "%", 0.3)
, mRecentFiles(recentFiles)
{
    addAndMakeVisible(mSliceTypeCombobox);
    mSliceTypeCombobox.comboBox.addItem("Div", 1);
    mSliceTypeCombobox.comboBox.addItem("Transient", 2);
    mSliceTypeCombobox.comboBox.addItem("Manual", 3);
    mSliceTypeCombobox.comboBox.setSelectedId(3);
    mSliceTypeCombobox.comboBox.onChange = [this]()
    {
        auto const idx = mSliceTypeCombobox.comboBox.getSelectedItemIndex();
        mAudioSource.setSliceMethod(static_cast<SliceManager::Method>(idx));
        
        mSliceDivsorSlider.setVisible(false);
        mSliceTransientThresholdSlider.setVisible(false);
        
        auto const sliceMethod = mAudioSource.getSliceManager().getSliceMethod();
        if(sliceMethod == SliceManager::Method::divisions)
        {
            mSliceDivsorSlider.setVisible(true);
        }
        else if(sliceMethod == SliceManager::Method::transients)
        {
            mSliceTransientThresholdSlider.setVisible(true);
        }
        else if(sliceMethod == SliceManager::Method::manual)
        {
            // ?
        }
    };
    
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
            
            // TODO: Handle this better
            mAudioSource.getSliceManager().sanitiseSlices();
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
    mSliceDivsorSlider.onValueChange = [this]()
    {
        auto const divisor = static_cast<int>(std::pow(2, static_cast<int>(mSliceDivsorSlider.getValue())));
        mAudioSource.setBlockDivisionFactor(divisor);
        mWaveformComponent.setSlicePositions(mAudioSource.getSliceManager().getSlices(), 0);
    };
    mSliceDivsorSlider.setValue(1.0, sendNotification);
    
    addChildComponent(mSliceTransientThresholdSlider);
    mSliceTransientThresholdSlider.mLabels.add({0.0, "0"});
    mSliceTransientThresholdSlider.mLabels.add({1.0, "1"});
    mSliceTransientThresholdSlider.setRange(0.0, 1.0, 0.1);
    mSliceTransientThresholdSlider.onValueChange = [this]()
    {
        auto const threshold = static_cast<float>(mSliceTransientThresholdSlider.getValue());
        mAudioSource.setTransientDetectionThreshold(threshold);
        mWaveformComponent.setSlicePositions(mAudioSource.getSliceManager().getSlices(), 0);
    };
    mSliceTransientThresholdSlider.setValue(0.3, sendNotification);
    
    addAndMakeVisible(mChangeSampleProbabilitySlider);
    mChangeSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mChangeSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mChangeSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mChangeSampleProbabilitySlider.onValueChange = [this]()
    {
        mSampleChangeThreshold = 1.0f - static_cast<float>(mChangeSampleProbabilitySlider.getValue());
        mAudioSource.setSampleChangeThreshold(mSampleChangeThreshold);
    };
    mChangeSampleProbabilitySlider.setValue(0.3, juce::NotificationType::sendNotification);
    
    addAndMakeVisible(mReverseSampleProbabilitySlider);
    mReverseSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mReverseSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mReverseSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mReverseSampleProbabilitySlider.onValueChange = [this]()
    {
        mReverseSampleThreshold = 1.0f - static_cast<float>(mReverseSampleProbabilitySlider.getValue());
        mAudioSource.setReverseSampleThreshold(mReverseSampleThreshold) ;
    };
    mReverseSampleProbabilitySlider.setValue(0.3, juce::NotificationType::sendNotification);
    
    addAndMakeVisible(mRetriggerSampleProbabilitySlider);
    mRetriggerSampleProbabilitySlider.mLabels.add({0.0, "0%"});
    mRetriggerSampleProbabilitySlider.mLabels.add({1.0, "100%"});
    mRetriggerSampleProbabilitySlider.setRange(0.0, 1.0, 0.1);
    mRetriggerSampleProbabilitySlider.onValueChange = [this]()
    {
        mRetriggerSampleThreshold = 1.0f - static_cast<float>(mRetriggerSampleProbabilitySlider.getValue());
        mAudioSource.setRetriggerSampleThreshold(mRetriggerSampleThreshold);
    };
    mRetriggerSampleProbabilitySlider.setValue(0.3, juce::NotificationType::sendNotification);
    
    addAndMakeVisible(mWaveformComponent);
    mWaveformComponent.onWaveformDoubleClicked = [this](int xPos)
    {
        // convert to slice position and set
        auto const bufferLength = mAudioSource.getSliceManager().getBufferNumSamples();
        auto const waveformSize = mWaveformComponent.getWidth();
        
        // convert to sample pos
        auto const samplePosition = static_cast<size_t>(xPos / static_cast<double>(waveformSize) * bufferLength);
        mAudioSource.getSliceManager().addSlice(samplePosition);
        
        if(mAudioSource.getSliceManager().getSliceMethod() != SliceManager::Method::manual)
        {
            mSliceTypeCombobox.comboBox.setSelectedItemIndex(static_cast<int>(SliceManager::Method::manual));
        }
    };
    
    mWaveformComponent.onSliceMarkerRightClicked = [this](int xPos)
    {
        // convert to slice position and set
        auto const bufferLength = mAudioSource.getSliceManager().getBufferNumSamples();
        auto const waveformSize = mWaveformComponent.getWidth();
        
        // convert width of 16 pixels to samples
        auto const sampleToleranceWidth = static_cast<int>(16 / static_cast<double>(waveformSize) * bufferLength);
        
        // convert to sample pos
        auto const samplePosition = static_cast<size_t>(xPos / static_cast<double>(waveformSize) * bufferLength);
        auto* slice = mAudioSource.getSliceManager().getSliceAtSamplePosition(samplePosition, sampleToleranceWidth);
        if(slice != nullptr)
        {
            mAudioSource.getSliceManager().deleteSlice(std::get<0>(*slice));
        }
    };
    
    mWaveformComponent.onSliceMarkerMouseDown = [this](int xPos)
    {
        // convert to slice position and set
        auto const bufferLength = mAudioSource.getSliceManager().getBufferNumSamples();
        auto const waveformSize = mWaveformComponent.getWidth();
        
        // convert width of 16 pixels to samples
        auto const sampleToleranceWidth = static_cast<int>(16 / static_cast<double>(waveformSize) * bufferLength);
        
        // convert to sample pos
        auto const samplePosition = static_cast<size_t>(xPos / static_cast<double>(waveformSize) * bufferLength);
        auto* slice = mAudioSource.getSliceManager().getSliceAtSamplePosition(samplePosition, sampleToleranceWidth);
        if(slice != nullptr)
        {
            mActiveMouseMarker = std::get<0>(*slice);
            mPrevDragPos = xPos;
        }
    };
    
    mWaveformComponent.onSliceMarkerDragged = [this](float xPos)
    {
        if(mActiveMouseMarker.isNull())
        {
            return;
        }
        
        auto* slice = mAudioSource.getSliceManager().getSliceById(mActiveMouseMarker);
        if(slice == nullptr)
        {
            return;
        }
        
        // convert to sample delta
        auto const delta_x = xPos - mPrevDragPos;
        auto const bufferLength = mAudioSource.getSliceManager().getBufferNumSamples();
        auto const waveformSize = mWaveformComponent.getWidth();
        auto const sampleDelta = static_cast<int>(delta_x / static_cast<double>(waveformSize) * bufferLength);
        
        // set
        mAudioSource.getSliceManager().moveSlice(std::get<0>(*slice), sampleDelta);
        mPrevDragPos = xPos;
    };
    
    mWaveformComponent.onMouseUp = [this]()
    {
        mActiveMouseMarker = juce::Uuid().null();
        mPrevDragPos = 0.0f;
    };
    
    addAndMakeVisible(mSampleLengthSeconds);
    mSampleLengthSeconds.setNumberOfDecimals(3);
    
    addAndMakeVisible(mSampleDesiredLengthSeconds);
    mSampleDesiredLengthSeconds.setNumberOfDecimals(3);
    mSampleDesiredLengthSeconds.setRange({0.1, 10.0}, juce::NotificationType::dontSendNotification);
    mSampleDesiredLengthSeconds.onValueChanged = [this](double value)
    {
        changeState(TransportState::Stopping);
        mPlayButton.setEnabled(false);
        
        auto const stretchFactor = static_cast<float>(value / mAudioSource.getSliceManager().getFileLength());
        auto const pitchFactor = static_cast<float>(std::pow(2.0, mPitchShiftSlider.getValue() / 12.0));
        mAudioSource.getSliceManager().performTimestretch(stretchFactor, pitchFactor, [this]()
        {
            mPlayButton.setEnabled(true);
            mAudioSource.getSliceManager().clearSlices();
            mAudioSource.getSliceManager().performSlice();
            mAudioSource.setNextReadPosition(0);
            updateWaveform();
        });
        
        // TODO: Handle this better
        mAudioSource.getSliceManager().sanitiseSlices();
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
    
    setSize (500, 620);

    mFormatManager.registerBasicFormats();
    
    mTransportSource.addChangeListener(this);
    mAudioSource.getSliceManager().addChangeListener(this);
    mWaveformComponent.getThumbnail().addChangeListener(this);
    
    mActiveMouseMarker = juce::Uuid().null();
    
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
    
    auto firstRowBounds = bounds.removeFromTop(20);
    mSliceTypeCombobox.setBounds(firstRowBounds.removeFromRight(static_cast<int>(threeFieldRowElementWidth * 1.5)));
    
    bounds.removeFromTop(20);
    
    auto secondRowBounds = bounds.removeFromTop(100);
    mPitchShiftSlider.setBounds(secondRowBounds.removeFromLeft(threeFieldRowElementWidth));
    secondRowBounds.removeFromLeft(threeFieldRowSpacing);
    mCrossFadeSlider.setBounds(secondRowBounds.removeFromLeft(threeFieldRowElementWidth));
    secondRowBounds.removeFromLeft(threeFieldRowSpacing);
    mSliceDivsorSlider.setBounds(secondRowBounds.removeFromLeft(threeFieldRowElementWidth));
    mSliceTransientThresholdSlider.setBounds(mSliceDivsorSlider.getBounds());
    
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
    else if(source == &mAudioSource.getSliceManager())
    {
        // update the waveform
        auto const& slices = mAudioSource.getSliceManager().getSlices();
        auto const activeSlice = mAudioSource.getSliceManager().getCurrentSliceIndex();
        
        std::cout << "Number of slices: " << slices.size() << "\n";
        
        mWaveformComponent.setSlicePositions(slices, activeSlice);
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
        
        // TODO: Handle this better
        mAudioSource.getSliceManager().sanitiseSlices();
    });
    
    mAudioSource.getSliceManager().clearSlices();
    changeState(TransportState::Stopped);
}

void BreakbeatContentComponent::newFileOpened(juce::String& filePath)
{
    mChosenPath.swapWith(filePath);
    notify();
}

void BreakbeatContentComponent::setFileOutputPath()
{
    mFileChooser = std::make_unique<juce::FileChooser>("Please select the location you'd like to record to...",                                                                                                           juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
                                                                                       "*.wav");
    
    auto folderChooserFlags = FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles;
    mFileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
    {
        auto file(chooser.getResult());
        mRecordedFile = chooser.getResult();
    });
}

void BreakbeatContentComponent::exportAudioSlices()
{
    // TODO: REIMPLEMENT THIS
//    juce::FileChooser fileChooser("Export slices to file(s)...", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.wav");
//    if(!fileChooser.browseForFileToSave(true))
//    {
//        return;
//    }
//
//    auto file = fileChooser.getResult();
//    try
//    {
////        auto* readBuffer = mAudioSource.getCurrentBuffer();
////        auto fileName = file.getFileNameWithoutExtension();
////        auto path = file.getParentDirectory().getFullPathName();
////        mSliceExporter.startExport(readBuffer, fileName, path, mAudioSource.getSliceSize(), mAudioSource.getNumSlices(), 2, 44100.0, 32);
//    }
//    catch (std::exception e)
//    {
//        juce::AlertWindow::showMessageBox(juce::AlertWindow::AlertIconType::WarningIcon, "Failed to export slices", e.what());
//    }
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
        juce::AlertWindow::showAsync(MessageBoxOptions()
                                     .withIconType (MessageBoxIconType::WarningIcon)
                                     .withTitle ("Load sample failed!")
                                     .withMessage (error)
                                     .withButton ("OK"), nullptr);
        return;
    }
    
    mRecentFiles.addFile(juce::File(pathToOpen));
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
    mWaveformComponent.getThumbnail().addBlock(0, *mAudioSource.getSliceManager().getForwardBuffer(), 0, static_cast<int>(mAudioSource.getSliceManager().getBufferNumSamples()));
}

void BreakbeatContentComponent::fromXml(juce::XmlElement const& xml)
{
    mAudioSource.getSliceManager().fromXml(xml);
}

std::unique_ptr<juce::XmlElement> BreakbeatContentComponent::toXml()
{
    return mAudioSource.getSliceManager().toXml();
}
