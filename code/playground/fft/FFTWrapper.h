#pragma once

#include "JuceHeader.h"

namespace OUS
{
    class FFTWrapper
    {
    public:
        // TODO: Fixed size for now, make it settable
        // TODO: Handling for multiple (/stereo?) channels
        FFTWrapper()
        : mMagnitudes(FFTSize, 0.0f)
        , mPhases(FFTSize, 0.0f)
        , mFrequencies(FFTSize, 0.0f)
        {
            
        }
        
        static constexpr auto FFTOrder = 11;
        static constexpr auto FFTSize = 1 << FFTOrder;
        
        const std::vector<float>& getMagnitudes() const;
        const std::vector<float>& getPhases() const;
        
        void performRealForwardTransform(float* data);
        void performRealInverseTransform(float* data);
        
    private:
        juce::dsp::FFT mFFT {FFTOrder};
        juce::dsp::WindowingFunction<float> mWindow { FFTSize, juce::dsp::WindowingFunction<float>::WindowingMethod::blackmanHarris };
        
        std::vector<float> mMagnitudes;
        std::vector<float> mPhases;
        std::vector<float> mFrequencies;
    };
}
