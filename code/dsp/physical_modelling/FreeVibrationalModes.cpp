#include "FreeVibrationalModes.h"

using namespace OUS;

FreeVibrationalModes::FreeVibrationalModes()
{
    mLowpassFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    mLowpassFilter.setCutoffFrequency(4000);
    
    for(auto& filter : mBandpassFilters)
    {
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    }
}

void FreeVibrationalModes::setFundamentalFrequency(float fundFrequency)
{
    for(size_t i = 0; i < NUM_FREE_HARMONICS; ++i)
    {
        auto const freq = fundFrequency * mHarmonicRatios[i];
        mBandpassFilters[i].setCutoffFrequency(freq < 0.5f * mSampleRate ? freq : 0.0f);
        mBandpassFilters[i].setResonance(1.0f);
    }
}

void FreeVibrationalModes::reset() noexcept
{
    mLowpassFilter.reset();
    
    for(auto& filter : mBandpassFilters)
    {
        filter.reset();
    }
}

void FreeVibrationalModes::prepare (const juce::dsp::ProcessSpec& spec)
{
    mSampleRate = static_cast<float>(spec.sampleRate);
    
    mLowpassFilter.prepare(spec);
    
    for(auto& filter : mBandpassFilters)
    {
        filter.prepare (spec);
    }
    
    for(size_t i = 0; i < NUM_FREE_HARMONICS; i++)
    {
        // This allocates the temp buffer memory too
        mTempBuffers[i] = juce::dsp::AudioBlock<float>(mTempBuffersMemory[i], spec.numChannels, spec.maximumBlockSize);
    }
}

void FreeVibrationalModes::process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept
{
    auto& outputBuffer = context.getOutputBlock();
    auto const numSamples = outputBuffer.getNumSamples();
    
    for(size_t s = 0; s < numSamples; ++s)
    {
        // generate white noise
        auto const noiseVal = random.nextFloat() * 2.0f - 1.0f;
        auto const outVal = outputBuffer.getSample(0, static_cast<int>(s));
        
        outputBuffer.setSample(0, static_cast<int>(s), noiseVal * outVal);
        outputBuffer.setSample(1, static_cast<int>(s), noiseVal * outVal);
    }
    
    // lpf mfreebarbuffer
    mLowpassFilter.process(context);
    
    for(size_t i = 0; i < NUM_FREE_HARMONICS; i++)
    {
        mTempBuffers[i].clear();
        mTempBuffers[i].copyFrom(outputBuffer);
        
        auto& filter = mBandpassFilters[i];
        auto filterContext = juce::dsp::ProcessContextReplacing<float>(mTempBuffers[i]);
        filter.process(filterContext);
    }
    
    // Sum all of the individual buffers and write to context output
    for(size_t s = 0; s < numSamples; ++s)
    {
        auto value = 0.0f;
        for(size_t i = 0; i < NUM_FREE_HARMONICS; ++i)
        {
            value += mTempBuffers[i].getSample(0, static_cast<int>(s));
        }
        
        outputBuffer.setSample(0, static_cast<int>(s), value);
        outputBuffer.setSample(1, static_cast<int>(s), value);
    }
}
