#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DopplerShiftPluginEditor.h"
#include "RubberbandPitchShifter.h"
#include <algorithm>

namespace OUS
{
    //==============================================================================
    class DopplerShiftProcessor
    : public juce::AudioProcessor
    , private juce::Timer
    {
    public:
        //==============================================================================
        DopplerShiftProcessor();

        //==============================================================================
        void prepareToPlay(double sampleRate, int samplesPerBlockExpected) override;
        void releaseResources() override;
        void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiBuffer) override;

        //==============================================================================
        AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override;

        //==============================================================================
        const String getName() const override;
        bool acceptsMidi() const override;
        bool producesMidi() const override;
        double getTailLengthSeconds() const override;

        //==============================================================================
        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram(int) override;
        const String getProgramName(int) override;
        void changeProgramName(int, const String&) override;

        //==============================================================================
        void getStateInformation(MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    private:
        void timerCallback() override;
        //==============================================================================

        juce::AudioProcessorValueTreeState mState;

        float static constexpr timerUpdateTime = 1000; // 1 second

        // TODO: Check the thread saftey of this parameter
        float mFrequencyRatio{1.0f};

        // Note in this way origin is defined as the center of the world (not the left top / bottom corner)!
        juce::Point<float> mSourcePosition{-30.0f, 0.0f};
        float mSourceDirection{1.0f};

        std::unique_ptr<RubberbandPitchShifter> mPitchShifter;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DopplerShiftProcessor)
    };
} // namespace OUS
