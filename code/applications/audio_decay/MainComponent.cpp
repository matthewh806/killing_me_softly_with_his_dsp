#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& audioDeviceManager)
: juce::AudioAppComponent(audioDeviceManager)
, mBitDepthSlider("Bit-Depth", "bit")
, mDownsamplingSlider("Downsampling", "x")
, mWetDrySlider("Mix", "")
{
    addAndMakeVisible(&mBitDepthSlider);
    mBitDepthSlider.mLabels.add({0.0f, "3"});
    mBitDepthSlider.mLabels.add({1.0f, "24"});
    mBitDepthSlider.setRange(3, 24, 1);
    mBitDepthSlider.setValue(16);
    mBitDepthSlider.onValueChange = [this]
    {
        mDecayProcessor.setQuantisationLevel(static_cast<int>(mBitDepthSlider.getValue()));
    };
    
    addAndMakeVisible(&mDownsamplingSlider);
    mDownsamplingSlider.mLabels.add({0.0f, "1"});
    mDownsamplingSlider.mLabels.add({1.0f, "10"});
    mDownsamplingSlider.setRange(1.0, 10.0, 1.0);
    mDownsamplingSlider.setValue(1);
    mDownsamplingSlider.onValueChange = [this]
    {
        mDecayProcessor.setDownsampleFactor(static_cast<int>(mDownsamplingSlider.getValue()));
    };
    
    addAndMakeVisible(&mWetDrySlider);
    mWetDrySlider.mLabels.add({0.0f, "Dry"});
    mWetDrySlider.mLabels.add({1.0f, "Wet"});
    mWetDrySlider.setRange(0.0, 1.0, 0.1);
    mWetDrySlider.setValue(0.3);
    mWetDrySlider.onValueChange = [this]
    {
        // set bit depth
        mDecayProcessor.setWetDryMix(static_cast<float>(mWetDrySlider.getValue()));
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
    
    mDecayProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    MidiBuffer midiBuffer;
    mDecayProcessor.processBlock(*bufferToFill.buffer, midiBuffer);
}

void MainComponent::releaseResources()
{
    mDecayProcessor.releaseResources();
}


//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduced(20, 20);
    
    auto constexpr numUIElements = 3;
    auto const rotaryWidth = static_cast<int>(bounds.getWidth() * 1 / numUIElements);
    auto const spacing = (bounds.getWidth() - rotaryWidth * numUIElements) / (numUIElements - 1);
    
    mBitDepthSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
    bounds.removeFromLeft(spacing);
    mDownsamplingSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
    bounds.removeFromLeft(spacing);
    mWetDrySlider.setBounds(bounds.removeFromLeft(rotaryWidth));
}
