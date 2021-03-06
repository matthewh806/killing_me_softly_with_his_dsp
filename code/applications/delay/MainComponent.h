#pragma once

#include "JuceHeader.h"
#include "../../dsp/processors/SimpleDelayProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent, juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent(juce::AudioDeviceManager& audioDeviceManager);
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    
    //==============================================================================
    RotarySliderWithLabels mDelayTimeSlider;
    RotarySliderWithLabels mWetDrySlider;
    RotarySliderWithLabels mFeedbackSlider;
    
    SimpleDelayProcessor mDelayProcessor;

    int mBlockSize;
    int mSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
