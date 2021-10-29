#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& activeDeviceManager)
: juce::AudioAppComponent(activeDeviceManager)
, juce::Thread("backgroundthread")
, mGrainDensitySlider("Grain density", "g/s")
, mGrainLengthSlider("Grain length", "ms")
, mGrainPositionRandomnessSlider("Position randomness", "")
, mGrainCountLabel("# grains:", "", 0, false)
, mSourceTypeSlider("Source type")
, mEnvelopeTypeSlider("Envelope type")
, mFrequencySlider("Frequency", "Hz")
{
    mFormatManager.registerBasicFormats();
    
    addAndMakeVisible(mGrainCountLabel);
    mGrainCountLabel.setNumberOfDecimals(0);
    
    addAndMakeVisible(mGrainDensitySlider);
    mGrainDensitySlider.setRange({1.0, 100.0}, 1.0);
    mGrainDensitySlider.mLabels.add({0.0, "1"});
    mGrainDensitySlider.mLabels.add({1.0, "100"});
    mGrainDensitySlider.setValue(1.0);
    mGrainDensitySlider.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            mScheduler->setGrainDensity(mGrainDensitySlider.getValue());
        }
    };
    
    addAndMakeVisible(mGrainLengthSlider);
    mGrainLengthSlider.setRange({10.0, 1000.0}, 1.0);
    mGrainLengthSlider.mLabels.add({0.0, "10"});
    mGrainLengthSlider.mLabels.add({1.0, "1000"});
    mGrainLengthSlider.setValue(300.0); // try to avoid killing our ears on startup
    mGrainLengthSlider.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            auto const lengthSeconds = mGrainLengthSlider.getValue() / 1000.0;
            mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));
        }
    };
    
    addAndMakeVisible(mGrainPositionRandomnessSlider);
    mGrainPositionRandomnessSlider.setRange({0.0, 1.0}, 0.05);
    mGrainPositionRandomnessSlider.mLabels.add({0.0, "0.0"});
    mGrainPositionRandomnessSlider.mLabels.add({1.0, "1.0"});
    mGrainPositionRandomnessSlider.setValue(0.0);
    mGrainPositionRandomnessSlider.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            mScheduler->setPositionRandomness(mGrainPositionRandomnessSlider.getValue());
        }
    };
    
    addAndMakeVisible(mSourceTypeSlider);
    mSourceTypeSlider.comboBox.addItem("Sample", 1);
    mSourceTypeSlider.comboBox.addItem("Synthetic", 2);
    mSourceTypeSlider.comboBox.setSelectedId(1);
    mSourceTypeSlider.comboBox.onChange = [this]()
    {
        auto const sourceType = static_cast<Source::SourceType>(mSourceTypeSlider.comboBox.getSelectedItemIndex());
        if(sourceType == mSourceType)
        {
            return;
        }
        
        mSourceType = sourceType;
        
        if(mScheduler != nullptr)
        {
            mScheduler->shouldSynthesise = false;
        }
        mScheduler = nullptr;
        switch(mSourceType)
        {
            case Source::SourceType::sample:
            {
                mWaveformComponent.setVisible(true);
                mFrequencySlider.setVisible(false);
                mGrainPositionRandomnessSlider.setVisible(true);
            }
            break;
            case Source::SourceType::synthetic:
            {
                mWaveformComponent.setVisible(false);
                mFrequencySlider.setVisible(true);
                mGrainPositionRandomnessSlider.setVisible(false);
                
                mScheduler = std::make_unique<Scheduler>(nullptr, mSourceType);
                if(mScheduler == nullptr)
                {
                    // todo: throw error
                    std::cerr << "Scheduler could not be created!\n";
                    return;
                }
                
                mScheduler->prepareToPlay(mBlockSize, mSampleRate);
                mScheduler->setGrainDensity(mGrainDensitySlider.getValue());
                auto const lengthSeconds = mGrainLengthSlider.getValue() / 1000.0;
                mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));
                mScheduler->setEnvelopeType(static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex()));
                mScheduler->shouldSynthesise = true;
            }
            break;
        }
        
        resized();
    };
    
    addAndMakeVisible(mEnvelopeTypeSlider);
    mEnvelopeTypeSlider.comboBox.addItem("Trapezoidal", 1);
    mEnvelopeTypeSlider.comboBox.addItem("Parabolic", 2);
    mEnvelopeTypeSlider.comboBox.setSelectedId(1);
    mEnvelopeTypeSlider.comboBox.onChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            mScheduler->setEnvelopeType(static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex()));
        }
    };
    
    addAndMakeVisible(mWaveformComponent);
    mWaveformComponent.onNewFileDropped = [this](juce::String& path)
    {
        juce::String err;
        loadSample(path, err);
    };
    
    addChildComponent(mFrequencySlider);
    mFrequencySlider.setRange({65.4, 1046.502}, 1);
    mFrequencySlider.mLabels.add({0.0, "C2"});
    mFrequencySlider.mLabels.add({1.0, "C6"});
    mFrequencySlider.setValue(220.0);
    mFrequencySlider.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            mScheduler->setOscillatorFrequency(mFrequencySlider.getValue());
        }
    };
    
    setSize (600, 360);
    startTimer(200);
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
}   

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
    
    if(mScheduler != nullptr)
    {
        mScheduler->prepareToPlay(mBlockSize, mSampleRate);
    }
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if(mScheduler != nullptr)
    {
        mScheduler->synthesise(bufferToFill.buffer, bufferToFill.numSamples);
    }
}

void MainComponent::releaseResources()
{
}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 20);
    
    auto rotaryBounds = bounds.removeFromTop(100);
    auto const threeColumnSliderWidth = static_cast<int>(rotaryBounds.getWidth() * 0.30f);
    auto const spacingWidth = (rotaryBounds.getWidth() - threeColumnSliderWidth*3)/2;
    
    mGrainDensitySlider.setBounds(rotaryBounds.removeFromLeft(threeColumnSliderWidth));
    rotaryBounds.removeFromLeft(spacingWidth);
    mGrainLengthSlider.setBounds(rotaryBounds.removeFromLeft(threeColumnSliderWidth));
    rotaryBounds.removeFromLeft(spacingWidth);
    mGrainPositionRandomnessSlider.setBounds(rotaryBounds.removeFromLeft(threeColumnSliderWidth));
    
    bounds.removeFromTop(10);
    auto comboBoxBounds = bounds.removeFromTop(30);
    auto const twoColumnSliderWidth = static_cast<int>(comboBoxBounds.getWidth() * 0.48f);
    auto const twoColumnSpacingWidth = comboBoxBounds.getWidth() - twoColumnSliderWidth *2;
    mSourceTypeSlider.setBounds(comboBoxBounds.removeFromLeft(twoColumnSliderWidth));
    comboBoxBounds.removeFromLeft(twoColumnSpacingWidth);
    mEnvelopeTypeSlider.setBounds(comboBoxBounds.removeFromLeft(twoColumnSliderWidth));
    
    mGrainCountLabel.setBounds(bounds.removeFromTop(40));
    
    bounds.removeFromTop(10);
    if(mSourceType == Source::SourceType::sample)
    {
        mWaveformComponent.setBounds(bounds.removeFromTop(100));
    }
    else if(mSourceType == Source::SourceType::synthetic)
    {
        mFrequencySlider.setBounds(bounds.removeFromTop(100).removeFromLeft(threeColumnSliderWidth));
    }
}

bool MainComponent::loadSample(juce::String const& filePath, juce::String& error)
{
    juce::ignoreUnused(error);
    
    juce::File file(filePath);
    mReader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor(file));
    if(mReader == nullptr)
    {
        return false;
    }
    
    auto const numChannels = static_cast<int>(mReader.get()->numChannels);
    auto const numSamples = static_cast<int>(mReader.get()->lengthInSamples);
    ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(file.getFileName(), numChannels, numSamples);
    mReader.get()->read(newBuffer->getAudioSampleBuffer(), 0, numSamples, 0, true, true);
    {
        const juce::SpinLock::ScopedLockType lock(mMutex);
        mCurrentBuffer = newBuffer;
    }
    
    mScheduler = std::make_unique<Scheduler>(mCurrentBuffer->getAudioSampleBuffer(), mSourceType);
    if(mScheduler == nullptr)
    {
        // todo: throw error
        std::cerr << "Scheduler could not be created!\n";
        return false;
    }
    
    mScheduler->prepareToPlay(mBlockSize, mSampleRate);
    mScheduler->setGrainDensity(mGrainDensitySlider.getValue());
    auto const lengthSeconds = mGrainLengthSlider.getValue() / 1000.0;
    mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));
    mScheduler->setEnvelopeType(static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex()));
    mScheduler->setPositionRandomness(mGrainPositionRandomnessSlider.getValue());
    
    mWaveformComponent.clear();
    
    mWaveformComponent.getThumbnail().reset(numChannels, numSamples);
    mWaveformComponent.getThumbnail().addBlock(0, *mCurrentBuffer->getAudioSampleBuffer(), 0, numSamples);
    mWaveformComponent.repaint();
    
    mScheduler->shouldSynthesise = true;
    
    return true;
}

void MainComponent::run()
{
    while(!threadShouldExit())
    {
        checkForBuffersToFree();
        wait(500);
    }
}

void MainComponent::checkForBuffersToFree()
{
    for (auto i = mBuffers.size(); --i >= 0;)
    {
        ReferenceCountedBuffer::Ptr buffer (mBuffers.getUnchecked (i));

        if (buffer->getReferenceCount() == 2)
        {
            mBuffers.remove(i);
        }
    }
}

void MainComponent::timerCallback()
{
    if(mScheduler != nullptr)
    {
        auto const grains = mScheduler->getNumberOfGrains();
        mGrainCountLabel.setValue(grains, juce::NotificationType::sendNotificationAsync);
    }
}
