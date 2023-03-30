#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/CustomLookAndFeel.h"
#include "ClampedVibrationalModes.h"
#include "FreeVibrationalModes.h"

namespace OUS
{
    /*
        This plugin models and synthesises the sound of a ruler being twanged
        on a desk. The resulting motion is quite complex, but can be roughly
        approximated as a combination of different vibrational excitations.
     
        This plugin models two vibrational modes:
            - Clamped (cantilevar) vibrations
            - Free bar vibrations
                - This excitation is additionally modulated & shaped by the
                  cantilevar vibrations
     
        Clamped Bar Vibrations:
            These vibrations are modelled by generating a phase locked sawtooth
            wave at the fundamental vibration frequency and using this to
            generate additional sawtooth overtones at specific multiples of the
            fundamental. These are used as input to a parallel set of cosine
            operators and summed together to give the resulting vibrational modes.
     
        Free Bar Vibrations:
            These vibrations are modelled on a white noise source which has been
            modulated / amplitude shaped by the clamped bar excitation and
            passed into a parallel bank of band pass filters which each have a
            center frequency tuned to an overtone multiple of the fundamental.
     
        These two vibrational modes are summed before being output.
     
        TODO:
            - Base the fundamental frequency on physical properties of the ruler
              (instead of directly inputting the Hz value)
            - Model Part length clamped vibrational modes (due to the ruler hitting
              the desk)
            - Vibrato on the fundamental frequency to simulate moving the ruler during
              the twang
            - Apply 16th power envelope to the envelope to give transient to the attack.
            - Use 4th order band pass filters for the noise src
            - Figure out what the magic numbers used in the Pd are doing
            - Presets for different ruler properties
     */
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
        float calculateFundamentalFrequency(float lambda);
        
        void resetSystem();
        void triggerSystem();
        
        int mBlockSize;
        int mSampleRate;
        
        std::unique_ptr<juce::AudioParameterBool> mTriggerTwang;
        juce::AudioParameterFloat* mDecayTime;
        
        juce::AudioParameterFloat* mYoungsModulus; // N/m^2
        juce::AudioParameterFloat* mRulerLength;   // mm
        juce::AudioParameterFloat* mRulerHeight;   // mm
        juce::AudioParameterFloat* mRulerDensity;  // kg / m^2
        
        juce::AudioParameterFloat* mFreeVibrationFrequency;
        juce::AudioParameterFloat* mClampedVibrationFrequency;
        
        juce::AudioProcessorValueTreeState mState;
        
        juce::dsp::Oscillator<float> mSawtoothRamp;
        juce::dsp::StateVariableTPTFilter<float> mLowpassFilter;
        juce::dsp::StateVariableTPTFilter<float> mHighpassFilter;
        
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
