#include "FFTWrapper.h"

using namespace OUS;

const std::vector<float>& FFTWrapper::getMagnitudes() const
{
    return mMagnitudes;
}

const std::vector<float>& FFTWrapper::getPhases() const
{
    return mPhases;
}

void FFTWrapper::performRealForwardTransform(float* data)
{
     mFFT.performRealOnlyForwardTransform(data);
    
    auto* complexOutput = reinterpret_cast<juce::dsp::Complex<float>*> (data);
    for (size_t i = 0; i < FFTSize; ++i)
    {
        mMagnitudes[i] = std::abs(complexOutput[i]);
        mPhases[i] = std::arg(complexOutput[i]);
    }
}

void FFTWrapper::performRealInverseTransform(float* data)
{
    mFFT.performRealOnlyInverseTransform(data);
}
