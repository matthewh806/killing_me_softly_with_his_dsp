#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.

#include "JuceHeader.h"
#include "../../dsp/processors/RealTimeStretchProcessor.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent
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
    void stretchValueChanged();
    void pitchShiftValueChanged();
    
    //==============================================================================
    juce::AudioDeviceSelectorComponent mAudioDeviceComponent;
    
    juce::Label mStretchFactorLabel;
    juce::Slider mStretchFactorSlider;
    
    juce::Label mPitchShiftLabel;
    juce::Slider mPitchShiftSlider;
    
    RealTimeStretchProcessor mStretchProcessor;
    
    int mBlockSize;
    int mSampleRate;
    
    int mMinfill = 0;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
