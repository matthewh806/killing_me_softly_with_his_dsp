#include "ClampedVibrationalModes.h"

using namespace OUS;

ClampedVibrationalModes::ClampedVibrationalModes(std::array<float, NUM_CLAMPED_HARMONICS> harmonicRatios)
: mHarmonicRatios(harmonicRatios)
{
    mADSR.setParameters({0.0f, 0.45f, 0.0f, 0.45f});
}

void ClampedVibrationalModes::setLevel(float linearValue)
{
    mGain.setGainLinear(linearValue);
}

void ClampedVibrationalModes::setDecayTime(float decayTimeMs)
{
    mADSR.setParameters({0.0f, decayTimeMs / 1000.0f, 0.0f, decayTimeMs / 1000.0f});
    mADSR.reset();
}

void ClampedVibrationalModes::trigger()
{
    mADSR.noteOn();
}

//==============================================================================
void ClampedVibrationalModes::reset() noexcept
{
    mGain.reset();
}

//==============================================================================
void ClampedVibrationalModes::prepare (const juce::dsp::ProcessSpec& spec)
{
    mADSR.setSampleRate(spec.sampleRate);
    mGain.prepare(spec);
    
    mADSR.reset();
}

//==============================================================================
void ClampedVibrationalModes::process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept
{
    auto& outputBuffer = context.getOutputBlock();
    
    auto const numSamples = outputBuffer.getNumSamples();
    for(size_t s = 0; s < numSamples; ++s)
    {
        auto const fundamentalValue = outputBuffer.getSample(0, static_cast<int>(s));
        auto summedValue = 0.0f;
        auto gain = 1.0f;
        for(size_t i = 0; i < NUM_CLAMPED_HARMONICS; ++i)
        {
            auto const freq = fundamentalValue * mHarmonicRatios[i];
            auto const cosValue = std::cos(freq * juce::MathConstants<float>::twoPi);
            summedValue += cosValue * gain;
            
            gain *= 0.5f;
        }
        
        auto const rampDownVal = mADSR.getNextSample();
        auto const quadRampVal = rampDownVal * rampDownVal * rampDownVal;
        summedValue *= quadRampVal;
        
        outputBuffer.setSample(0, static_cast<int>(s), summedValue);
        outputBuffer.setSample(1, static_cast<int>(s), summedValue);
    }
    
    mGain.process(context);
}
