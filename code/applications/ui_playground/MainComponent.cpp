#include "MainComponent.h"
#include "BinaryData.h"


//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& deviceManager)
: juce::AudioAppComponent(deviceManager)
{
    setSize (600, 250);
    setAudioChannels (2, 2);
    
    addAndMakeVisible(mTestButton);
    mTestButton.setImages(true,
                          true,
                          true,
                          juce::ImageCache::getFromMemory(BinaryData::sync_button1_png, BinaryData::sync_button1_pngSize),
                          1.0f,
                          juce::Colours::transparentBlack,
                          juce::ImageCache::getFromMemory(BinaryData::sync_button1_png, BinaryData::sync_button1_pngSize),
                          1.0f,
                          juce::Colours::transparentBlack,
                          juce::ImageCache::getFromMemory(BinaryData::sync_button2_png, BinaryData::sync_button2_pngSize),
                          1.0f,
                          juce::Colours::transparentBlack);
    
    mTestButton.setClickingTogglesState(true);
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
    bufferToFill.clearActiveBufferRegion();
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
    mTestButton.setBounds(bounds.removeFromTop(60));
}
