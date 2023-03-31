#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "FreeVibrationalModes.h"
#include "ClampedVibrationalModes.h"

namespace OUS
{
    class RulerVibrationalModes
    {
    public:
        RulerVibrationalModes();
        
        void setLevel(float linearValue);
        void setDecayTime(float decayTimeMs);
        
        float getFreeFundamentalFrequency();
        float getClampedFundamentalFrequency();
        
        void setFundamentalFrequency(float youngsModulus, float height, float length, float density);
        void setFundamentalFrequency(float frequencyHz);
        
        void triggerSystem();
        
        void reset() noexcept;
        void prepare (const juce::dsp::ProcessSpec& spec);
        
        void process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept;
        
    private:
        
        juce::dsp::Oscillator<float> mSawtoothRamp;
        juce::dsp::StateVariableTPTFilter<float> mHighpassFilter;
        
        ClampedVibrationalModes mFullClampedModes;
        FreeVibrationalModes mFreeVibrationModes;
        
        juce::AudioBuffer<float> mSawtoothRampBuffer;
        juce::AudioBuffer<float> mClampedBarBuffer;
        juce::AudioBuffer<float> mFreeBarBuffer;
        
        float mFreeFundamentalFrequency;
        float mClampedFundamentalFrequency;
        
        juce::dsp::Gain<float> mGain;
    };
}
