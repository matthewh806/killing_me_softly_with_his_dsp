#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: mAudioDeviceComponent(deviceManager, 0, 256, 0, 256, false, false, false, true)
, mDelayTimeSlider("Delay Time", "s")
, mWetDrySlider("Wet / Dry", "")
, mFeedbackSlider("Feedback", "%")
{
    addAndMakeVisible(mAudioDeviceComponent);
    
    addAndMakeVisible(&mDelayTimeSlider);
    mDelayTimeSlider.mLabels.add({0.0f, "0.1s"});
    mDelayTimeSlider.mLabels.add({1.0f, "2s"});
    mDelayTimeSlider.setRange(0.1, 2, 0.1);
    mDelayTimeSlider.setValue(1.0);
    mDelayTimeSlider.onValueChange = [this]
    {
        mDelayProcessor.setDelayTime(mDelayTimeSlider.getValue());
    };
    
    addAndMakeVisible(&mWetDrySlider);
    mWetDrySlider.mLabels.add({0.0f, "0"});
    mWetDrySlider.mLabels.add({1.0f, "1"});
    mWetDrySlider.setRange(0, 1, 0.1);
    mWetDrySlider.setValue(0.5);
    mWetDrySlider.onValueChange = [this]
    {
        mDelayProcessor.setWetDryMix(mWetDrySlider.getValue());
    };
    
    addAndMakeVisible(&mFeedbackSlider);
    mFeedbackSlider.mLabels.add({0.0f, "0%"});
    mFeedbackSlider.mLabels.add({1.0f, "100%"});
    mFeedbackSlider.setRange(0, 100, 1);
    mFeedbackSlider.setValue(50);
    mFeedbackSlider.onValueChange = [this]
    {
        mDelayProcessor.setFeedback(mFeedbackSlider.getValue() / 100.0f);
    };

    setSize (600, 400);
    setAudioChannels (2, 2);
    
    deviceManager.addChangeListener(this);
}

MainComponent::~MainComponent()
{
    deviceManager.removeChangeListener(this);
    shutdownAudio();
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
    
    mDelayProcessor.prepareToPlay(sampleRate, samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    MidiBuffer midiBuffer;
    mDelayProcessor.processBlock(*bufferToFill.buffer, midiBuffer);
}

void MainComponent::releaseResources()
{
    mDelayProcessor.releaseResources();
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
    auto const rotaryWidth = bounds.getWidth() * 0.30;
    auto const spacing = (bounds.getWidth() - (rotaryWidth * 3)) / 2.0;
    mDelayTimeSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
    bounds.removeFromLeft(spacing);
    mWetDrySlider.setBounds(bounds.removeFromLeft(rotaryWidth));
    bounds.removeFromLeft(spacing);
    mFeedbackSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if(source == &deviceManager)
    {
        std::cout << deviceManager.getAudioDeviceSetup().inputDeviceName << "\n";
    }
}
