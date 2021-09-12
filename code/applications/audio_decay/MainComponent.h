#pragma once

#include "JuceHeader.h"
#include "../../dsp/processors/AudioDecayProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent
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
    //==============================================================================
    
    AudioDecayProcessor mDecayProcessor;
    
    RotarySliderWithLabels mBitDepthSlider;
    RotarySliderWithLabels mDownsamplingSlider;
    RotarySliderWithLabels mWetDrySlider;
    
    float mQuantisationLevel = 1.0;

    int mBlockSize;
    int mSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
