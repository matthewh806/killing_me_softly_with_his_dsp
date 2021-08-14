#pragma once

#include "JuceHeader.h"
#include "../../core/CircularBuffer.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent, juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    
    //==============================================================================
    juce::AudioDeviceSelectorComponent mAudioDeviceComponent;
    
    juce::Random random;
    
    juce::Label mDelayTimeLabel;
    juce::Slider mDelayTimeSlider;
    
    juce::Label mWetDryLabel;
    juce::Slider mWetDrySlider;
    
    juce::Label mFeedbackLabel;
    juce::Slider mFeedbackSlider;
    
    CircularBuffer<float>** mDelayBuffers;

    int mBlockSize;
    int mSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
