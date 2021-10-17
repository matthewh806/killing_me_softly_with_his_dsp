#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& activeDeviceManager)
: juce::AudioAppComponent(activeDeviceManager)
, juce::Thread("backgroundthread")
, mGrainDensity("Grain density", "g/s")
, mGrainLength("Grain length", "ms")
, mGrainCountLabel("# grains:", "", 0, false)
{
    mFormatManager.registerBasicFormats();
    
    addAndMakeVisible(mGrainCountLabel);
    mGrainCountLabel.setNumberOfDecimals(0);
    
    addAndMakeVisible(mGrainDensity);
    mGrainDensity.setRange({1.0, 100.0}, 1.0);
    mGrainDensity.mLabels.add({0.0, "1"});
    mGrainDensity.mLabels.add({1.0, "100"});
    mGrainDensity.setValue(1.0);
    mGrainDensity.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            mScheduler->setGrainDensity(mGrainDensity.getValue());
        }
    };
    
    addAndMakeVisible(mGrainLength);
    mGrainLength.setRange({10.0, 1000.0}, 1.0);
    mGrainLength.mLabels.add({0.0, "10"});
    mGrainLength.mLabels.add({1.0, "1000"});
    mGrainLength.setValue(1.0);
    mGrainLength.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            auto const lengthSeconds = mGrainLength.getValue() / 1000.0;
            mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));
        }
    };
    
    addAndMakeVisible(mWaveformComponent);
    mWaveformComponent.onNewFileDropped = [this](juce::String& path)
    {
        juce::String err;
        loadSample(path, err);
    };
    
    setSize (600, 300);
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
    auto const twoColumnSliderWidth = static_cast<int>(rotaryBounds.getWidth() * 0.45f);
    auto const spacingWidth = rotaryBounds.getWidth() - twoColumnSliderWidth * 2;
    
    mGrainDensity.setBounds(rotaryBounds.removeFromLeft(twoColumnSliderWidth));
    rotaryBounds.removeFromLeft(spacingWidth);
    mGrainLength.setBounds(rotaryBounds.removeFromLeft(twoColumnSliderWidth));
    
    mGrainCountLabel.setBounds(bounds.removeFromTop(40));
    
    bounds.removeFromTop(20);
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
    ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(file.getFileName(), numChannels, numSamples);                                                       ;
    mReader.get()->read(newBuffer->getAudioSampleBuffer(), 0, numSamples, 0, true, true);
    {
        const juce::SpinLock::ScopedLockType lock(mMutex);
        mCurrentBuffer = newBuffer;
    }
    
    mScheduler = std::make_unique<Scheduler>(mCurrentBuffer->getAudioSampleBuffer());
    mScheduler->prepareToPlay(mBlockSize, mSampleRate);
    
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
