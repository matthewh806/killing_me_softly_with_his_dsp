/*
  ==============================================================================

    BreakbeatMaker.cpp
    Created: 4 Jul 2020 5:39:51pm
    Author:  Matthew

  ==============================================================================
*/

#include "BreakbeatMaker.h"
#include "BinaryData.h"

BreakbeatContentComponent::BreakbeatContentComponent(juce::AudioDeviceManager& audioDeviceManager, juce::RecentlyOpenedFilesList& recentFiles)
: juce::AudioAppComponent(audioDeviceManager)
, juce::Thread("Background Thread")
, mPitchShiftSlider("Pitch shift", "")
, mCrossFadeSlider("Cross fade", "ms", 10.0)
, mSliceDivsorSlider("Slice Div", "", 1.0)
, mSliceTransientThresholdSlider("Detection Thresh.", "", 0.3)
, mChangeSampleProbabilitySlider("Swap slice", "%", 0.3)
, mReverseSampleProbabilitySlider("Reverse slice", "%", 0.3)
, mRetriggerSampleProbabilitySlider("Retrigger slice", "%", 0.3)
, mRecentFiles(recentFiles)
{
    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Retro Gaming");
    
    Font titleFont(getLookAndFeel().getLabelFont (mApplicationTitle));
    titleFont.setHeight(42);
    titleFont.setTypefaceName("Retro Gaming");
    mApplicationTitle.setFont(titleFont);
    addAndMakeVisible(mApplicationTitle);
    
    Font probabilityFont(getLookAndFeel().getLabelFont(mProbabilityTitle));
    probabilityFont.setHeight(35);
    mProbabilityTitle.setFont(probabilityFont);
    addAndMakeVisible(mProbabilityTitle);
    
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
    mCrossFadeSlider.mLabels.add({1.0, "30"});
    mCrossFadeSlider.setRange(0.0, 30.0, 1.0);
    mCrossFadeSlider.setValue(10.0, juce::NotificationType::dontSendNotification);
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
    
    mWaveformComponent.onNewFileDropped = [this](juce::String& path)
    {
        newFileOpened(path);
    };
    
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
//    mSampleDesiredLengthSeconds.setRange({0.1, 10.0}, juce::NotificationType::dontSendNotification);
//    mSampleDesiredLengthSeconds.onValueChanged = [this](double value)
//    {
//        changeState(TransportState::Stopping);
//        mPlayButton.setEnabled(false);
//
//        auto const stretchFactor = static_cast<float>(value / mAudioSource.getSliceManager().getFileLength());
//        auto const pitchFactor = static_cast<float>(std::pow(2.0, mPitchShiftSlider.getValue() / 12.0));
//        mAudioSource.getSliceManager().performTimestretch(stretchFactor, pitchFactor, [this]()
//        {
//            mPlayButton.setEnabled(true);
//            mAudioSource.getSliceManager().clearSlices();
//            mAudioSource.getSliceManager().performSlice();
//            mAudioSource.setNextReadPosition(0);
//            updateWaveform();
//        });
//        
//        // TODO: Handle this better
//        mAudioSource.getSliceManager().sanitiseSlices();
//    };
//    
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
    mApplicationTitle.setBounds(80, 23, 421, 78);
    mProbabilityTitle.setBounds(50, 155, 178, 38);
    
    mReverseSampleProbabilitySlider.setBounds(44, 198, 92, 92);
    mRetriggerSampleProbabilitySlider.setBounds(134, 198, 92, 92);
    mChangeSampleProbabilitySlider.setBounds(44, 302, 92, 92);
    
    mPitchShiftSlider.setBounds(267, 192, 92, 92);
    mCrossFadeSlider.setBounds(382, 192, 92, 92);
    mSliceTypeCombobox.setBounds(267, 319, 108, 42);
    mSliceDivsorSlider.setBounds(382, 295, 92, 92);
    
    mFileNameLabel.setBounds(55, 439, 200, 19);
    mSampleLengthSeconds.setBounds(310, 439, 50, 19);
    mSampleDesiredLengthSeconds.setBounds(395, 439, 80, 19);
    mWaveformComponent.setBounds(44, 454, 418, 121);
}

void BreakbeatContentComponent::paint(juce::Graphics& g)
{
    auto backgroundImg = juce::ImageCache::getFromMemory(BinaryData::breakbeat_machine_ui_png, BinaryData::breakbeat_machine_ui_pngSize);
    g.drawImage(backgroundImg, getLocalBounds().toFloat());
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
    mWaveformComponent.setSampleRate(static_cast<float>(sampleRate));
    
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
    if(source == &mTransportSource)
    {
        changeState(mTransportSource.isPlaying() ? TransportState::Playing : TransportState::Stopped);
    }
    else if(source == &mAudioSource.getSliceManager())
    {
        // update the waveform
        auto const& slices = mAudioSource.getSliceManager().getSlices();
        auto const activeSlice = mAudioSource.getSliceManager().getCurrentSliceIndex();
        
//        std::cout << "Number of slices: " << slices.size() << "\n";
        
        mWaveformComponent.setSlicePositions(slices, activeSlice);
    }
}

void BreakbeatContentComponent::handleAsyncUpdate()
{
    mFileNameLabel.setText(mAudioSource.getSliceManager().getSampleFileName(), juce::NotificationType::dontSendNotification);
    mFileSampleRateLabel.setText(juce::String(mAudioSource.getSliceManager().getSampleSampleRate()), juce::NotificationType::dontSendNotification);
    
    mSampleLengthSeconds.setText(juce::String(mAudioSource.getSliceManager().getFileLength()), juce::NotificationType::sendNotification);
    
    auto const pitchFactor = static_cast<float>(std::pow(2.0, mPitchShiftSlider.getValue() / 12.0));
    mAudioSource.getSliceManager().performTimestretch(1.0f, pitchFactor, [this]()
    {
        mSampleDesiredLengthSeconds.setText(juce::String(mAudioSource.getSliceManager().getBufferLength()), juce::NotificationType::dontSendNotification);
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

bool BreakbeatContentComponent::keyPressed(juce::KeyPress const& key)
{
    if(key == KeyPress::spaceKey) // r
    {
        changeState(mState == TransportState::Starting || mState == TransportState::Playing
            ? TransportState::Stopping : TransportState::Starting);
    }
    
    return true;
}

bool BreakbeatContentComponent::keyPressed (const juce::KeyPress& key, juce::Component* originatingComponent)
{
    juce::ignoreUnused(originatingComponent);
    return keyPressed(key);
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
