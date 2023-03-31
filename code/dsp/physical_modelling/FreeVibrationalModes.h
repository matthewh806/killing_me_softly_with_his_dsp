#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

namespace OUS
{
    #define NUM_FREE_HARMONICS 5
    #define LAMBDA_FREE_FUNDAMENTAL 4.7300407f

    /*
     This class models the vibrational modes of an excited free bar
     
     It does so by taking a modulated white noise source and passing
     this through a parallel bank of bandpass filters each of which has
     its center frequency tuned to one of the overtone multiples of the
     fundamental frequency of the excitation.
     
     Note:
        The input to this method is expected to be a modulated white noise
        source. In addition to any audio modulation the modulation source
        should apply a volume envelope to the sound since none is done internally
        by this class.
     
        Otherwise the noise will sound out forever!
     
     */
    class FreeVibrationalModes
    {
    public:
        FreeVibrationalModes();
        
        void setFundamentalFrequency(float fundFrequency);
        
        void reset() noexcept;
        void prepare (const juce::dsp::ProcessSpec& spec);
        
        void process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept;
        
    private:
        // we create these because the processing is done in parallel on the buffers
        std::array<juce::HeapBlock<char>, NUM_FREE_HARMONICS> mTempBuffersMemory;
        std::array<juce::dsp::AudioBlock<float>, NUM_FREE_HARMONICS> mTempBuffers;
        
        juce::dsp::StateVariableTPTFilter<float> mLowpassFilter;
        
        const std::array<float, NUM_FREE_HARMONICS> mHarmonicRatios { 1.0f, 2.7565f, 5.4039f, 8.9330f, 13.3443f };
        std::array<juce::dsp::StateVariableTPTFilter<float>, NUM_FREE_HARMONICS> mBandpassFilters;
        
        float mSampleRate {44100.0f};
        
        juce::Random random;
    };
}
