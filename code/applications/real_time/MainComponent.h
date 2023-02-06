#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.

#include "JuceHeader.h"
#include "../../dsp/processors/RealTimeStretchProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
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
        void stretchValueChanged();
        void pitchShiftValueChanged();
        
        //==============================================================================
        
        RotarySliderWithLabels mStretchFactorSlider;
        RotarySliderWithLabels mPitchShiftSlider;
        
        RealTimeStretchProcessor mStretchProcessor;
        
        int mBlockSize;
        int mSampleRate;
        
        int mMinfill = 0;
        

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
