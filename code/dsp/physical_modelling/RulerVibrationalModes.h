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
        
        /*
         This sets the mix ratio of free mode vs clamp mode
         vibrations in the final output.
         The value should be in the inclusive range 0,1
            
            balance = 0 : only free vibrational modes
            balance = 1 : only clamped vibrational modes
            balance = 0.5 : equal mix between the two modes
         */
        void setVibrationalModeBalance(float balance);
        
        /*
         This sets a flag checked by the internal processor to
         determine whether or not to modulate the free bar
         vibrations with an external signal.
         
         Setting this to true means that in the process method a modulating
         signal can be generated and passed to the FreeVibrationalModes
         processing method (via the context)
         */
        void setModulateFreeVibrationalModes(bool modulate);
        
        float getFreeFundamentalFrequency();
        float getClampedFundamentalFrequency();
        
        /*
         This allows the fundamental frequency of vibration to be calculated and set based
         on a set of physical ruler properties.
         */
        void setFundamentalFrequency(float youngsModulus, float height, float length, float density);
        
        /*
         This allows the fundamental frequency of the vibration to be set directly as a Hz value
         
         Note:
            The free bar vibrations directly use this fundamental frequency
            The cantilever bar takes frequency Hz / 4.0f as its fundamental (c.f. Pd patch)
         */
        void setFundamentalFrequency(float frequencyHz);
        
        /*
         Resets and retriggers the whole system. 
         */
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
        
        // TODO: atomic?
        bool mModulateFreeVibrationalModes { true };
        float mFreeToClampedMixRatio {0.5f};
        
        juce::dsp::Gain<float> mGain;
    };
}
