#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    #define NUM_HARMONICS 5

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
        ClampedVibrationalModes(std::array<float, NUM_HARMONICS> harmonicRatios)
        : mHarmonicRatios(harmonicRatios)
        {
            mADSR.setParameters({0.0f, 0.45f, 0.0f, 0.45f});
        }
        
        void setLevel(float linearValue)
        {
            mGain.setGainLinear(linearValue);
        }
        
        void setDecayTime(float decayTimeMs)
        {
            mADSR.setParameters({0.0f, decayTimeMs / 1000.0f, 0.0f, decayTimeMs / 1000.0f});
            mADSR.reset();
        }
        
        void trigger()
        {
            mADSR.noteOn();
        }
        
        //==============================================================================
        void reset() noexcept
        {
            mGain.reset();
        }

        //==============================================================================
        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            mADSR.setSampleRate(spec.sampleRate);
            mGain.prepare(spec);
            
            mADSR.reset();
        }
        
        //==============================================================================
        template <typename ProcessContext>
        void process (const ProcessContext& context) noexcept
        {
            auto& outputBuffer = context.getOutputBlock();
            
            auto const numSamples = outputBuffer.getNumSamples();
            for(size_t s = 0; s < numSamples; ++s)
            {
                auto const fundamentalValue = outputBuffer.getSample(0, static_cast<int>(s));
                auto summedValue = 0.0f;
                auto gain = 1.0f;
                for(size_t i = 0; i < NUM_HARMONICS; ++i)
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
        
    private:
        
        std::array<float, NUM_HARMONICS> mHarmonicRatios;
        
        juce::ADSR mADSR;
        
        juce::dsp::Gain<float> mGain;
    };

    class FreeVibrationalModes
    {
    public:
        FreeVibrationalModes()
        {
            for(auto& filter : mBandpassFilters)
            {
                filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            }
        }
        
        void setFundamentalFrequency(float fundFrequency)
        {
            for(size_t i = 0; i < NUM_HARMONICS; ++i)
            {
                auto const freq = fundFrequency * mHarmonicRatios[i];
                mBandpassFilters[i].setCutoffFrequency(freq);
            }
        }
        
        void reset() noexcept
        {
            for(auto& filter : mBandpassFilters)
            {
                filter.reset();
            }
        }
        
        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            for(auto& filter : mBandpassFilters)
            {
                filter.prepare (spec);
            }
            
            for(size_t i = 0; i < NUM_HARMONICS; i++)
            {
                // This allocates the temp buffer memory too
                mTempBuffers[i] = juce::dsp::AudioBlock<float>(mTempBuffersMemory[i], spec.numChannels, spec.maximumBlockSize);
            }
        }
        
        template <typename ProcessContext>
        void process (const ProcessContext& context) noexcept
        {
            auto& outputBuffer = context.getOutputBlock();
            auto const numSamples = outputBuffer.getNumSamples();
            
            for(size_t i = 0; i < NUM_HARMONICS; i++)
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
                auto gain = 0.5f;
                auto value = 0.0f;
                for(size_t i = 0; i < NUM_HARMONICS; ++i)
                {
                    value += mTempBuffers[i].getSample(0, static_cast<int>(s)) * gain;
                    gain *= 0.5f;
                }
                
                outputBuffer.setSample(0, static_cast<int>(s), value);
                outputBuffer.setSample(1, static_cast<int>(s), value);
            }
        }
    private:
        // we create these because the processing is done in parallel on the buffers
        std::array<juce::HeapBlock<char>, NUM_HARMONICS> mTempBuffersMemory;
        std::array<juce::dsp::AudioBlock<float>, NUM_HARMONICS> mTempBuffers;
        
        const std::array<float, NUM_HARMONICS> mHarmonicRatios { 1.0f, 2.7565f, 5.4039f, 8.9330f, 13.3443f };
        std::array<juce::dsp::StateVariableTPTFilter<float>, NUM_HARMONICS> mBandpassFilters;
    };

    //==============================================================================
    class RulerTwangPlugin
    : public juce::AudioProcessor
    , public juce::AudioProcessorValueTreeState::Listener
    {
    public:
        //==============================================================================
        RulerTwangPlugin();
        ~RulerTwangPlugin() override;

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        const String getName() const override { return "RulerTwangPlugin"; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const String getProgramName(int) override { return "None"; }
        void changeProgramName(int, const String&) override {}
        bool isVST2() const noexcept { return (wrapperType == wrapperType_VST); }

        //==============================================================================
        void getStateInformation(MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;
        
        //==============================================================================
        void parameterChanged(const juce::String& parameterID, float newValue) override;

    private:
        //==============================================================================
        int mBlockSize;
        int mSampleRate;
        
        juce::AudioParameterBool* mTriggerTwang;
        juce::AudioParameterFloat* mDecayTime;
        juce::AudioParameterFloat* mVibrationFrequency;
        
        juce::AudioProcessorValueTreeState mState;
        
        juce::dsp::Oscillator<float> mSawtoothRamp;
        ClampedVibrationalModes mFullClampedModes;
        FreeVibrationalModes mFreeVibrationModes;
        
        juce::AudioBuffer<float> mSawtoothRampBuffer;
        juce::AudioBuffer<float> mClampedBarBuffer;
        juce::AudioBuffer<float> mFreeBarBuffer;
        
        juce::Random random;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RulerTwangPlugin)
    };
} // namespace OUS
