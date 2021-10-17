#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& deviceManager)
: juce::AudioAppComponent(deviceManager)
{
    mFormatManager.registerBasicFormats();
    
    addAndMakeVisible(mWaveformComponent);
    mWaveformComponent.onNewFileDropped = [this](juce::String& path)
    {
        juce::String err;
        loadSample(path, err);
    };
    
    setSize (600, 250);
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}   

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
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
    
    g.setColour (juce::Colours::darkblue);
    g.setFont (20.0f);
    g.drawText ("Some people just do nothing", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 20);
    mWaveformComponent.setBounds(bounds.removeFromBottom(100));
}

bool MainComponent::loadSample(juce::String const& filePath, juce::String& error)
{
    juce::File file(filePath);
    mReader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor(file));
    mScheduler = std::make_unique<Scheduler>(*mReader.get());
    
    mWaveformComponent.clear();
    
    auto const numChannels = static_cast<int>(mReader.get()->numChannels);
    auto const numSamples = static_cast<int>(mReader.get()->lengthInSamples);
    auto const sampleRate = mReader.get()->sampleRate;
                                              
    juce::AudioBuffer<float> buff {numChannels, numSamples};
    mReader.get()->read(&buff, 0, numSamples, 0, true, true);
    
    mWaveformComponent.getThumbnail().reset(numChannels, numSamples);
    mWaveformComponent.getThumbnail().addBlock(0, buff, 0, numSamples);
    mWaveformComponent.repaint();
    
    mScheduler->shouldSynthesise = true;
    
    return true;
}
