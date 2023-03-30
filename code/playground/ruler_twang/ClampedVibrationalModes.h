#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

namespace OUS
{
    #define NUM_CLAMPED_HARMONICS 5

    /*
        This class models the vibrational modes of a ruler clamped at one end.
        
        It does so by using cosine operators to generate partials (including
        the fundamental) for frequencies scaled by numerically determined
        harmonic ratios
     
        The cosines are phase locked to the fundamental and have a quadratic
        decaying ramp applied to them.
     
        Note: The input to the process method is expected to be an audio rate
              signal between 0 & 1 which will be multiplied by 2*pi
              and passed into a cosine operator to give values in the range [-1,1]
     
              This input signal could be for e.g. a linear ramp from 0 -> 1
              at a given frequenncy. This is supposed to be used as input
              for other oscillators rather than to be listened to.
                
              It's how the phase sync. with the fundamental is achieved.
     
              This input buffer is not calculated internally because it may be
              used to modulate other signals outside of this class too
     */

    class ClampedVibrationalModes
    {
    public:
        ClampedVibrationalModes(std::array<float, NUM_CLAMPED_HARMONICS> harmonicRatios);
        
        void setLevel(float linearValue);
        void setDecayTime(float decayTimeMs);
        
        void trigger();
        
        void reset() noexcept;
        void prepare (const juce::dsp::ProcessSpec& spec);
        
        void process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept;
        
    private:
        
        std::array<float, NUM_CLAMPED_HARMONICS> mHarmonicRatios;
        
        juce::ADSR mADSR;
        
        juce::dsp::Gain<float> mGain;
    };
}
