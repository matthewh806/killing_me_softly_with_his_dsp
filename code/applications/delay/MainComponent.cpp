#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: mAudioDeviceComponent(deviceManager, 0, 256, 0, 256, false, false, false, true)
{
    addAndMakeVisible(mAudioDeviceComponent);
    
    addAndMakeVisible(&mDelayTimeSlider);
    mDelayTimeSlider.setRange(0.1, 2, 0.1);
    mDelayTimeSlider.setValue(1.0);
    mDelayTimeSlider.onValueChange = [this]
    {
        mDelayProcessor.setDelayTime(mDelayTimeSlider.getValue());
    };
    
    addAndMakeVisible(&mDelayTimeLabel);
    mDelayTimeLabel.setText("Delay Time (s)", juce::NotificationType::dontSendNotification);
    mDelayTimeLabel.attachToComponent(&mDelayTimeSlider, true);
    
    addAndMakeVisible(&mWetDrySlider);
    mWetDrySlider.setRange(0, 1, 0.1);
    mWetDrySlider.setValue(0.5);
    mWetDrySlider.onValueChange = [this]
    {
        mDelayProcessor.setWetDryMix(mWetDrySlider.getValue());
    };
    
    addAndMakeVisible(&mWetDryLabel);
    mWetDryLabel.setText("Wet/Dry Mix", juce::NotificationType::dontSendNotification);
    mWetDryLabel.attachToComponent(&mWetDrySlider, true);
    
    addAndMakeVisible(&mFeedbackLabel);
    mFeedbackLabel.setText("Feedback %", juce::NotificationType::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, true);
    
    addAndMakeVisible(&mFeedbackSlider);
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
    
    auto delayTimeBounds = bounds.removeFromTop(20);
    mDelayTimeLabel.setBounds(delayTimeBounds.removeFromLeft(100));
    mDelayTimeSlider.setBounds(delayTimeBounds);
    
    bounds.removeFromTop(10);
    
    auto wetDryBounds = bounds.removeFromTop(20);
    mWetDryLabel.setBounds(wetDryBounds.removeFromLeft(100));
    mWetDrySlider.setBounds(wetDryBounds);
    
    bounds.removeFromTop(10);
    
    auto feedbackBounds = bounds.removeFromTop(20);
    mFeedbackLabel.setBounds(feedbackBounds.removeFromLeft(100));
    mFeedbackSlider.setBounds(feedbackBounds);
}

void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if(source == &deviceManager)
    {
        std::cout << deviceManager.getAudioDeviceSetup().inputDeviceName << "\n";
    }
}
