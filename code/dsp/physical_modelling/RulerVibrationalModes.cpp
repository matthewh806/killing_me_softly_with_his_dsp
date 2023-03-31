#include "RulerVibrationalModes.h"

using namespace OUS;

RulerVibrationalModes::RulerVibrationalModes()
: mFullClampedModes({1.0f, 6.2669f, 17.5475f, 34.3861f, 56.8426f})
{
    mSawtoothRamp.initialise([](float x) { return juce::jmap(x, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f, 1.0f); }, 128);
    
    mHighpassFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    mHighpassFilter.setCutoffFrequency(90);
    
    mFullClampedModes.setLevel(0.2f);
}

void RulerVibrationalModes::setLevel(float linearValue)
{
    mFullClampedModes.setLevel(linearValue);
}

void RulerVibrationalModes::setDecayTime(float decayTimeMs)
{
    mFullClampedModes.setDecayTime(decayTimeMs);
}

void RulerVibrationalModes::setVibrationalModeBalance(float balance)
{
    mFreeToClampedMixRatio = std::clamp(balance, 0.0f, 1.0f);
}

void RulerVibrationalModes::setModulateFreeVibrationalModes(bool modulate)
{
    mModulateFreeVibrationalModes = modulate;
}

float RulerVibrationalModes::getFreeFundamentalFrequency()
{
    return mFreeFundamentalFrequency;
}

float RulerVibrationalModes::getClampedFundamentalFrequency()
{
    return mClampedFundamentalFrequency;
}

void RulerVibrationalModes::setFundamentalFrequency(float youngsModulus, float height, float length, float density)
{
    auto calculateFreq = [&](float lambda)
    {
        auto constexpr oneOverSquareRootTwelve = 0.2886751346f;
        auto const kappa = oneOverSquareRootTwelve * height;
        
        return std::sqrt(youngsModulus * kappa * kappa / density) * lambda * lambda / (length * length * juce::MathConstants<float>::twoPi);
    };
    
    mFreeFundamentalFrequency = calculateFreq(LAMBDA_FREE_FUNDAMENTAL);
    mClampedFundamentalFrequency = calculateFreq(LAMBDA_CLAMPED_FUNDAMENTAL);
    
    mFreeVibrationModes.setFundamentalFrequency(mFreeFundamentalFrequency);
    mSawtoothRamp.setFrequency(mClampedFundamentalFrequency);
}

void RulerVibrationalModes::setFundamentalFrequency(float frequencyHz)
{
    mFreeVibrationModes.setFundamentalFrequency(frequencyHz);
    mSawtoothRamp.setFrequency(frequencyHz / 4.0f); // this magic number is from the pd patch
}

void RulerVibrationalModes::triggerSystem()
{
    // This class has an internal ADSR which essentially
    // retriggers the whole system
    reset();
    mFullClampedModes.trigger();
}

void RulerVibrationalModes::reset() noexcept
{
    mSawtoothRamp.reset();
    mHighpassFilter.reset();
    mFullClampedModes.reset();
    mFreeVibrationModes.reset();
}

void RulerVibrationalModes::prepare (const juce::dsp::ProcessSpec& spec)
{
    mSawtoothRamp.prepare(spec);
    mSawtoothRamp.setFrequency(220.0f);
    
    mHighpassFilter.prepare(spec);
    
    mFullClampedModes.prepare(spec);
    
    mFreeVibrationModes.prepare(spec);
    mFreeVibrationModes.setFundamentalFrequency(220.0f);
    
    mSawtoothRampBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
    mClampedBarBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
    mFreeBarBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
}

void RulerVibrationalModes::process (const juce::dsp::ProcessContextReplacing<float>& context) noexcept
{
    auto outputBlock = context.getOutputBlock();
    outputBlock.clear();
    
    mSawtoothRampBuffer.clear();
    mClampedBarBuffer.clear();
    mFreeBarBuffer.clear();
    
    juce::dsp::AudioBlock<float> sawtoothRampBlock(mSawtoothRampBuffer, 0);
    mSawtoothRamp.process(juce::dsp::ProcessContextReplacing<float>(sawtoothRampBlock));
    
    juce::dsp::AudioBlock<float> clampedBlock(mClampedBarBuffer, 0);
    clampedBlock.copyFrom(sawtoothRampBlock);
    mFullClampedModes.process(juce::dsp::ProcessContextReplacing<float>(clampedBlock));
    
    auto const numSamples = static_cast<int>(outputBlock.getNumSamples());
    
    if(mModulateFreeVibrationalModes)
    {
        for(auto i = 0; i < numSamples; ++i)
        {
            // generate value to modulate free bar excitation with
            auto const squaredRampVal = sawtoothRampBlock.getSample(0, i) * sawtoothRampBlock.getSample(0, i);
            auto const modulatedClampedVal = clampedBlock.getSample(0, i) * squaredRampVal;
            
            // modulate with clamped vibration values
            mFreeBarBuffer.setSample(0, i, modulatedClampedVal);
            mFreeBarBuffer.setSample(1, i, modulatedClampedVal);
        }
    }
    else
    {
        // set all the values equal to 1 which means the free bar vibrations wont be shaped / modulated at all
        for(auto i = 0; i < numSamples; ++i)
        {
            mFreeBarBuffer.setSample(0, i, 1.0f);
            mFreeBarBuffer.setSample(1, i, 1.0f);
        }
    }
    
    juce::dsp::AudioBlock<float> freeBlock(mFreeBarBuffer, 0);
    juce::dsp::ProcessContextReplacing<float> freeBlockProcessingContext(freeBlock);
    
    // pass to free bar vibration processor
    mFreeVibrationModes.process(freeBlockProcessingContext);
    
    // sum the two modes and output!
    for(auto i = 0; i < numSamples; ++i)
    {
        auto const value = (1.0f - mFreeToClampedMixRatio) * mFreeBarBuffer.getSample(0, i) + mFreeToClampedMixRatio * mClampedBarBuffer.getSample(0, i);
        outputBlock.setSample(0, i, value);
        outputBlock.setSample(1, i, value);
    }
    
    // hpf buffer
    mHighpassFilter.process(juce::dsp::ProcessContextReplacing<float>(outputBlock));
}
