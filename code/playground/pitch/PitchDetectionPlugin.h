#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../dsp/processors/PitchDetectionProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    //==============================================================================
    class PitchDetectionPlugin
    : public juce::AudioProcessor
    , private juce::Timer
    {
    public:
        //==============================================================================
        PitchDetectionPlugin();

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return new PitchDetectionPluginEditor(*this); }
        bool hasEditor() const override { return true; }
        const String getName() const override { return "PitchDetectionPlugin"; }
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
        class PitchDetectionPluginEditor
        : public juce::AudioProcessorEditor
        {
        public:
            PitchDetectionPluginEditor(PitchDetectionPlugin& owner)
            : juce::AudioProcessorEditor(owner)
            {
                setSize(650, 400);
            }

            ~PitchDetectionPluginEditor() override
            {
            }

            //==============================================================================
            void setPitch(float pitch)
            {
                mLastDetectedPitch = pitch;
                repaint();
            }

            //==============================================================================
            void paint(juce::Graphics& g) override
            {
                // (Our component is opaque, so we must completely fill the background with a solid colour)
                g.setColour(juce::Colours::white);
                g.fillAll();
                g.setColour(juce::Colours::black);

                auto font = g.getCurrentFont();
                font.setHeight(100);
                g.setFont(font);

                auto const bounds = getLocalBounds();
                g.drawText(juce::String(mLastDetectedPitch, 1) + " Hz", bounds, juce::Justification::centred);
            }

            void resized() override
            {
                auto bounds = getLocalBounds();
                bounds.removeFromTop(20);
            }

        private:
            float mLastDetectedPitch{0.0f};
        };

        //==============================================================================
        int mBlockSize;
        int mSampleRate;

        juce::AudioProcessorValueTreeState mState;
        PitchDetectionProcessor mPitchDetectionProcessor;

        aubio_pitch_t* mAudioPitch = nullptr;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchDetectionPlugin)
    };
} // namespace OUS
