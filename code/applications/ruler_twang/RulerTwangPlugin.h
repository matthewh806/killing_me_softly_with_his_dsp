#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/CustomLookAndFeel.h"
#include "../../dsp/physical_modelling/RulerVibrationalModes.h"
#include "RulerTwangPluginEditor.h"

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
        juce::AudioProcessorEditor* createEditor() override
        {
            return new RulerTwangPluginProcessorEditor(*this, mState,
            [&]()
            {
                triggerSystem();
            },
            [&](juce::String presetName)
            {
                loadPreset(presetName);
            },
            [&]()
            {
                savePreset();
            });
        }
        
        bool hasEditor() const override { return true; }
        const String getName() const override { return "RulerTwangPlugin"; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return "None"; }
        
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
        
        juce::File getPresetsFolder();
        void savePreset();
        void loadPreset(juce::String presetName);
        
        std::unique_ptr<juce::FileChooser> mFileChooser = nullptr;
        
        int mBlockSize;
        int mSampleRate;
        
        int mCurrentProgram {0};
        
        juce::AudioProcessorValueTreeState mState;
        
        RulerVibrationalModes mRulerVibrationalModes;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RulerTwangPlugin)
    };
} // namespace OUS
