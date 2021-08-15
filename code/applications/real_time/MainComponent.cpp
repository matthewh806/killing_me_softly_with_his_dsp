#include "MainComponent.h"
#include <RubberBandStretcher.h>

//==============================================================================
MainComponent::MainComponent()
: mAudioDeviceComponent(deviceManager, 0, 256, 0, 256, false, false, false, true)
{
    addAndMakeVisible(mAudioDeviceComponent);
    
    addAndMakeVisible(&mStretchFactorSlider);
    mStretchFactorSlider.setRange(0.1, 4, 0.1);
    mStretchFactorSlider.setValue(1.0);
    mStretchFactorSlider.onValueChange = [this] { stretchValueChanged(); };
    
    addAndMakeVisible(&mStretchFactorLabel);
    mStretchFactorLabel.setText("Stretch Factor", juce::NotificationType::dontSendNotification);
    mStretchFactorLabel.attachToComponent(&mStretchFactorSlider, true);
    
    addAndMakeVisible(&mPitchShiftSlider);
    mPitchShiftSlider.setRange(0.1, 10, 0.1);
    mPitchShiftSlider.setValue(1.0);
    mPitchShiftSlider.onValueChange = [this] { pitchShiftValueChanged(); };
    
    addAndMakeVisible(&mPitchShiftLabel);
    mPitchShiftLabel.setText("Pitch Shift", juce::NotificationType::dontSendNotification);
    mPitchShiftLabel.attachToComponent(&mPitchShiftSlider, true);
    
    setSize (600, 400);
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    
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
    mAudioDeviceComponent.setBounds(bounds.removeFromTop(getHeight() / 2));
    
    bounds.removeFromTop(20);
    
    auto stretchFactorBounds = bounds.removeFromTop(20);
    mStretchFactorLabel.setBounds(stretchFactorBounds.removeFromLeft(100));
    mStretchFactorSlider.setBounds(stretchFactorBounds);
    
    bounds.removeFromTop(10);
    
    auto pitchFactorBounds = bounds.removeFromTop(20);
    mPitchShiftLabel.setBounds(pitchFactorBounds.removeFromLeft(100));
    mPitchShiftSlider.setBounds(pitchFactorBounds);
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
