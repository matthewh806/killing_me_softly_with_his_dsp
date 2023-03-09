#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../core/MultitapCircularBuffer.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    //==============================================================================
    class MultitapDelayPlugin
    : public juce::AudioProcessor
    , private juce::Timer
    {
    public:
        //==============================================================================
        MultitapDelayPlugin();

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return nullptr; }
        bool hasEditor() const override { return false; }
        const String getName() const override { return "MultitapDelayPlugin"; }
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

        // juce::Timer
        //==============================================================================
        void timerCallback() override;

    private:
        //==============================================================================
        juce::AudioParameterFloat* mTapADelayTime;
        juce::AudioParameterFloat* mTapBDelayTime;
        juce::AudioParameterFloat* mFeedbackA;
        juce::AudioParameterFloat* mFeedbackB;
        juce::AudioParameterFloat* mWetDryMix;
        
        int mBlockSize;
        int mSampleRate;

        juce::AudioProcessorValueTreeState mState;
        
        // Stored in seconds (converted in audio processor)
        std::vector<float> mTapDelays {2.0, 1.3f};
        std::vector<std::unique_ptr<MultitapCircularBuffer<float>>> mDelayBuffers;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultitapDelayPlugin)
    };
} // namespace OUS
