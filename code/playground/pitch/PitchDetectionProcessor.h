#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "aubio.h"
#include "../../ui/CustomLookAndFeel.h"

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
    class PitchDetectionProcessor : public juce::AudioProcessor
    {
    public:
        // TODO: Make this private
        juce::AudioProcessorValueTreeState state;

        //==============================================================================
        PitchDetectionProcessor();

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return new PitchDetectionProcessorEditor(*this); }
        bool hasEditor() const override { return true; }
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
        class PitchDetectionProcessorEditor
        : public juce::AudioProcessorEditor
        , private juce::AsyncUpdater
        , private juce::AudioProcessorValueTreeState::Listener
        {
        public:
            PitchDetectionProcessorEditor(PitchDetectionProcessor& owner)
            : juce::AudioProcessorEditor(owner)
            {
                owner.state.addParameterListener("detectedpitch", this);
                setSize(650, 400);
            }

            ~PitchDetectionProcessorEditor() override
            {
//                state.removeParameterListener("detectedpitch", this);
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
            
            //==============================================================================
            void handleAsyncUpdate() override
            {
                repaint();
            }
            
            //==============================================================================
            void parameterChanged (const String& parameterID, float newValue) override
            {
                juce::ignoreUnused(parameterID, newValue);
                if(parameterID == "detectedpitch")
                {
                    mLastDetectedPitch = newValue;
                    triggerAsyncUpdate();
                }
            }

        private:
            float mLastDetectedPitch{0.0f};
        };

        //==============================================================================
        int mBlockSize;
        int mSampleRate;
        
        aubio_pitch_t* mAudioPitch;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchDetectionProcessor)
    };
} // namespace OUS
