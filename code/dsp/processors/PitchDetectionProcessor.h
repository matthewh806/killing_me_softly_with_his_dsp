#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "aubio.h"

// 1. prepare audio thread
//      create aubio objects before entering the audio processing thread
//      delete these objects when the audio thread exits
// 2. update audio thread
//      in your audio processing function, copy (or point) the samples coming from Qt into an aubio fvect_t
//      still in your audio processing function, execute the aubio_..._do function
// 3. do something with the results!

namespace OUS
{
    //==============================================================================
    class PitchDetectionProcessor
    : public juce::AudioProcessor
    {
    public:
        //==============================================================================
        PitchDetectionProcessor();
        
        //==============================================================================
        float getMostRecentPitch() const;

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        const String getName() const override { return "PitchDetection"; }
        bool acceptsMidi() const override { return false; }
        bool producesMidi() const override { return false; }
        double getTailLengthSeconds() const override { return 0.0; }
        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const String getProgramName(int) override { return {}; }
        void changeProgramName(int, const String&) override {}
        bool isVST2() const noexcept { return (wrapperType == wrapperType_VST); }

        //==============================================================================
        void getStateInformation(MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

    private:
        //==============================================================================
        int mBlockSize;
        int mSampleRate;
        
        juce::AudioProcessorValueTreeState mState;
        aubio_pitch_t* mAudioPitch = nullptr;
        
        fvec_t* mInputSamples = nullptr;
        fvec_t* mOutputVector = nullptr;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchDetectionProcessor)
    };
} // namespace OUS
