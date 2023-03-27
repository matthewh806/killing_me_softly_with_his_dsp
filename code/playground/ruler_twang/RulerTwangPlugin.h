#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    #define NUM_HARMONICS 5

    class ClampedVibrationalModes
    {
    public:
        ClampedVibrationalModes(std::array<float, NUM_HARMONICS> harmonicRatios)
        : mHarmonicRatios(harmonicRatios)
        {
            for(auto& osc : mCosineOscs)
            {
                osc.initialise([](float x) { return std::cos(x); }, 128);
            }
            
            mADSR.setParameters({0.0f, 0.45f, 0.0f, 0.45f});
        }
        
        void setFundamentalFrequency(float fundFrequency)
        {
            for(size_t i = 0; i < NUM_HARMONICS; ++i)
            {
                auto const freq = fundFrequency * mHarmonicRatios[i];
                mCosineOscs[i].setFrequency(freq, true);
            }
            
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
            // silence existing?
            mADSR.noteOn();
        }
        
        //==============================================================================
        void reset() noexcept
        {
            for(auto& osc : mCosineOscs)
            {
                osc.reset();
            }
            
            mGain.reset();
        }

        //==============================================================================
        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            for(auto& osc : mCosineOscs)
            {
                osc.prepare (spec);
            }
            
            for(size_t i = 0; i < NUM_HARMONICS; i++)
            {
                // This allocates the temp buffer memory too
                mTempBuffers[i] = juce::dsp::AudioBlock<float>(mTempBuffersMemory[i], spec.numChannels, spec.maximumBlockSize);
            }
            
            mADSR.setSampleRate(spec.sampleRate);
            mGain.prepare(spec);
            
            mADSR.reset();
        }
        
        //==============================================================================
        template <typename ProcessContext>
        void process (const ProcessContext& context) noexcept
        {
            // process in PARALLEL!
            for(size_t i = 0; i < NUM_HARMONICS; i++)
            {
                mTempBuffers[i].clear();
                auto& osc = mCosineOscs[i];
                auto oscContext = juce::dsp::ProcessContextReplacing<float>(mTempBuffers[i]);
                osc.process(oscContext);
            }
            
            // Sum all of the individual buffers and write to context output
            auto& outputBuffer = context.getOutputBlock();
            for(size_t s = 0; s < outputBuffer.getNumSamples(); ++s)
            {
                auto gain = 1.0f;
                auto value = 0.0f;
                for(size_t i = 0; i < NUM_HARMONICS; ++i)
                {
                    value += mTempBuffers[i].getSample(0, static_cast<int>(s)) * gain;
                    gain *= 0.5f;
                }
                
                auto const rampDownVal = mADSR.getNextSample();
                auto const quadRampVal = rampDownVal * rampDownVal * rampDownVal;
                
                value *= quadRampVal;
                outputBuffer.setSample(0, static_cast<int>(s), value);
                outputBuffer.setSample(1, static_cast<int>(s), value);
            }
            
            mGain.process(context);
        }
        
    private:
        // we create these because the processing is done in parallel on the buffers
        std::array<juce::HeapBlock<char>, NUM_HARMONICS> mTempBuffersMemory;
        std::array<juce::dsp::AudioBlock<float>, NUM_HARMONICS> mTempBuffers;
        
        std::array<float, NUM_HARMONICS> mHarmonicRatios;
        std::array<juce::dsp::Oscillator<float>, NUM_HARMONICS> mCosineOscs;
        
        juce::ADSR mADSR;
        
        juce::dsp::Gain<float> mGain;
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
        
        ClampedVibrationalModes mFullClampedModes;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RulerTwangPlugin)
    };
} // namespace OUS
