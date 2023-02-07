#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../core/RingBuffer.h"
#include "../../dependencies/rubberband/rubberband/RubberBandStretcher.h"

namespace OUS
{
    //==============================================================================
    class RealTimeStretchProcessor : public juce::AudioProcessor
    {
    public:
        //==============================================================================
        RealTimeStretchProcessor();

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        // TODO: Do it properly - this is not thread safe...
        void setStretchFactor(float newValue);
        void setPitchShift(float newValue);

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return new juce::GenericAudioProcessorEditor(*this); }
        bool hasEditor() const override { return true; }
        const String getName() const override { return "SimpleDelay"; }
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
        AudioParameterFloat* mStretchFactor;
        AudioParameterFloat* mPitchShift;

        std::unique_ptr<RubberBand::RubberBandStretcher> mRubberBand;

        juce::AudioSampleBuffer mInput;
        juce::AudioSampleBuffer mScratchBuffer;
        RubberBand::RingBuffer<float>** mOutputBuffer;

        int mBlockSize;
        int mSampleRate;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RealTimeStretchProcessor)
    };
} // namespace OUS
