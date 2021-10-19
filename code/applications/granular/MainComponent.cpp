#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& activeDeviceManager)
: juce::AudioAppComponent(activeDeviceManager)
, juce::Thread("backgroundthread")
, mGrainDensitySlider("Grain density", "g/s")
, mGrainLengthSlider("Grain length", "ms")
, mGrainPositionRandomnessSlider("Position randomness", "")
, mGrainCountLabel("# grains:", "", 0, false)
, mEnvelopeTypeSlider("Envelope type")
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
    mEnvelopeTypeSlider.setBounds(bounds.removeFromTop(30));
    mGrainCountLabel.setBounds(bounds.removeFromTop(40));
    
    bounds.removeFromTop(10);
    mWaveformComponent.setBounds(bounds.removeFromTop(100));
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
    
    mScheduler = std::make_unique<Scheduler>(mCurrentBuffer->getAudioSampleBuffer());
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
