#include "MainComponent.h"
#include <RubberBandStretcher.h>

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& deviceManager)
: juce::AudioAppComponent(deviceManager)
, mStretchFactorSlider("Stretch Factor", "x")
, mPitchShiftSlider("Pitch Shift Factor", "x")
{
    addAndMakeVisible(&mStretchFactorSlider);
    mStretchFactorSlider.mLabels.add({0.0, "0.1x"});
    mStretchFactorSlider.mLabels.add({1.0, "4.0x"});
    mStretchFactorSlider.setRange(0.1, 4, 0.1);
    mStretchFactorSlider.setValue(1.0);
    mStretchFactorSlider.onValueChange = [this] { stretchValueChanged(); };
    
    addAndMakeVisible(&mPitchShiftSlider);
    mPitchShiftSlider.mLabels.add({0.0, "0.1x"});
    mPitchShiftSlider.mLabels.add({1.0, "4.0x"});
    mPitchShiftSlider.setRange(0.1, 10, 0.1);
    mPitchShiftSlider.setValue(1.0);
    mPitchShiftSlider.onValueChange = [this] { pitchShiftValueChanged(); };
    
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
    
    mStretchProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    MidiBuffer midiBuffer;
    mStretchProcessor.processBlock(*bufferToFill.buffer, midiBuffer);
}

void MainComponent::releaseResources()
{
    mStretchProcessor.releaseResources();
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
    bounds.reduce(20, 20);
    
    auto const sliderWidth = bounds.getWidth() * 0.4;
    mStretchFactorSlider.setBounds(bounds.removeFromLeft(sliderWidth));
    bounds.removeFromLeft(bounds.getWidth() * 0.2);
    mPitchShiftSlider.setBounds(bounds.removeFromLeft(sliderWidth));
}

void MainComponent::stretchValueChanged()
{
    // TODO: Not thread safe...
    mStretchProcessor.setStretchFactor(mStretchFactorSlider.getValue());
}

void MainComponent::pitchShiftValueChanged()
{
    mStretchProcessor.setPitchShift(mPitchShiftSlider.getValue());
}
