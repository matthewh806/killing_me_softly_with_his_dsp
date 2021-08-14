#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
: mAudioDeviceComponent(deviceManager, 0, 256, 0, 256, false, false, false, true)
{
    addAndMakeVisible(mAudioDeviceComponent);
    
    addAndMakeVisible(&mDelayTimeSlider);
    mDelayTimeSlider.setRange(0.1, 2, 0.1);
    mDelayTimeSlider.setValue(1.0);
    mDelayTimeSlider.onValueChange = [this] { };
    
    addAndMakeVisible(&mDelayTimeLabel);
    mDelayTimeLabel.setText("Delay Time (s)", juce::NotificationType::dontSendNotification);
    mDelayTimeLabel.attachToComponent(&mDelayTimeSlider, true);
    
    addAndMakeVisible(&mWetDrySlider);
    mWetDrySlider.setRange(0, 1, 0.1);
    mWetDrySlider.setValue(0.5);
    mWetDrySlider.onValueChange = [this] { };
    
    addAndMakeVisible(&mWetDryLabel);
    mWetDryLabel.setText("Wet/Dry Mix", juce::NotificationType::dontSendNotification);
    mWetDryLabel.attachToComponent(&mWetDrySlider, true);
    
    addAndMakeVisible(&mFeedbackLabel);
    mFeedbackLabel.setText("Feedback %", juce::NotificationType::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, true);
    
    addAndMakeVisible(&mFeedbackSlider);
    mFeedbackSlider.setRange(0, 100, 1);
    mFeedbackSlider.setValue(50);
    mFeedbackSlider.onValueChange = [this] { };

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
    
    mDelayBuffers = new CircularBuffer<float> *[2];
    auto const bufferSize = static_cast<int>(2.0 * sampleRate) + 1; // avoid round down?
    mDelayBuffers[0] = new CircularBuffer<float>();
    mDelayBuffers[0]->createCircularBuffer(bufferSize);
    mDelayBuffers[1] = new CircularBuffer<float>();
    mDelayBuffers[1]->createCircularBuffer(bufferSize);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
//    bufferToFill.clearActiveBufferRegion();
    
    auto* device = deviceManager.getCurrentAudioDevice();
    auto const inputChannels = device->getActiveInputChannels();
    auto const outputChannels = device->getActiveOutputChannels();
    
    auto const delayTime = mDelayTimeSlider.getValue();
    auto const wetDryRatio = mWetDrySlider.getValue();
    auto const feedbackAmt = mFeedbackSlider.getValue() / 100.0;
    
    auto const channels = bufferToFill.buffer->getNumChannels();
    if(channels > 2)
    {
        return;
    }
    
    auto const numSamples = bufferToFill.numSamples;
    for(auto ch = 0; ch < channels; ++ch)
    {
        for(int sample = 0; sample < numSamples; ++sample)
        {
            auto const inputSignal = bufferToFill.buffer->getSample(ch, sample);
            auto const delayedSample = mDelayBuffers[ch]->readBuffer(delayTime * mSampleRate); // not thread safe...

            double inputToDelayBuffer = inputSignal + feedbackAmt * delayedSample;
            mDelayBuffers[ch]->writeBuffer(inputToDelayBuffer);
            bufferToFill.buffer->setSample(ch, sample, (1.0 - wetDryRatio) * inputSignal + wetDryRatio * delayedSample);
        }
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
