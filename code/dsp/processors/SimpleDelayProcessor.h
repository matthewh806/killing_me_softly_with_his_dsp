#pragma once

// clang-format off
#include "JuceHeader.h"
#include "PixelArtBinaryData.h"
#include "SimpleDelayBinaryData.h"
// clang-format on

#include "../../core/CircularBuffer.h"
#include "../../ui/CustomLookAndFeel.h"
#include "../../ui/PixelArtLookAndFeel.h"

namespace OUS
{
    //==============================================================================
    class SimpleDelayProcessor : public juce::AudioProcessor
    {
    public:
        // TODO: Make this private
        juce::AudioProcessorValueTreeState state;
        constexpr static float syncedDelayDivisions[7] = {1, 2, 4, 8, 16, 32, 64};

        //==============================================================================
        SimpleDelayProcessor();

        //==============================================================================
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay(double, int) override;
        void releaseResources() override;
        void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

        //==============================================================================
        // TODO: Do it properly - this is not thread safe...
        void setWetDryMix(float newValue);
        void setDelayTime(float newValue);
        void setFeedback(float newValue);

        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override { return new SimpleDelayPluginProcessorEditor(*this); }
        bool hasEditor() const override { return true; }
        const String getName() const override { return "SimpleDelay"; }
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

    private:
        //==============================================================================
        class SimpleDelayPluginProcessorEditor
        : public juce::AudioProcessorEditor
        , private juce::AudioProcessorValueTreeState::Listener
        {
        public:
            SimpleDelayPluginProcessorEditor(SimpleDelayProcessor& owner)
            : juce::AudioProcessorEditor(owner)
            , mSyncToggle()
            , mDelayTimeSlider("Delay Time", "s")
            , mSyncedDelaySlider("Beat Fraction", "")
            , mWetDrySlider("Wet / Dry", "")
            , mFeedbackSlider("Feedback", "")
            , mSyncToggleAttachment(owner.state, "sync", mSyncToggle)
            , mDelayTimeAttachment(owner.state, "delaytime", mDelayTimeSlider)
            , mSyncedDelayTimeAttachment(owner.state, "delaydivisor", mSyncedDelaySlider)
            , mWetDryAttachment(owner.state, "wetdry", mWetDrySlider)
            , mFeedbackAttachment(owner.state, "feedback", mFeedbackSlider)
            {
                addAndMakeVisible(mTitleLabel);
                mTitleLabel.setText("Simple Delay", juce::NotificationType::dontSendNotification);
                mTitleLabel.setEditable(false);
                mTitleLabel.setColour(juce::Label::textColourId, juce::Colours::black);
                mTitleLabel.setJustificationType(juce::Justification::centred);
                Font titleFont;
                titleFont.setHeight(34);
                titleFont.setTypefaceName("Retro Gaming");
                mTitleLabel.setFont(titleFont);
                
                addAndMakeVisible(mSyncToggle);
                owner.state.addParameterListener("sync", this);

                addAndMakeVisible(&mDelayTimeSlider);
                mDelayTimeSlider.mLabels.add({0.0f, "0.1s"});
                mDelayTimeSlider.mLabels.add({1.0f, "2s"});

                addAndMakeVisible(&mSyncedDelaySlider);
                mSyncedDelaySlider.mLabels.add({0.0f, "1"});
                mSyncedDelaySlider.mLabels.add({1.0f, "1/64"});

                addAndMakeVisible(&mWetDrySlider);
                mWetDrySlider.mLabels.add({0.0f, "0"});
                mWetDrySlider.mLabels.add({1.0f, "1"});

                addAndMakeVisible(&mFeedbackSlider);
                mFeedbackSlider.mLabels.add({0.0f, "0%"});
                mFeedbackSlider.mLabels.add({1.0f, "100%"});

                mDelayTimeSlider.setVisible(true);
                mSyncedDelaySlider.setVisible(false);

                setSize(600, 250);
            }

            ~SimpleDelayPluginProcessorEditor() override
            {
                dynamic_cast<SimpleDelayProcessor&>(processor).state.removeParameterListener("sync", this);
            }

            //==============================================================================
            void paint(juce::Graphics& g) override
            {
                auto backgroundImg = juce::ImageCache::getFromMemory(SimpleDelayBinaryData::simple_delay_background_png, SimpleDelayBinaryData::simple_delay_background_pngSize);
                g.drawImage(backgroundImg, getLocalBounds().toFloat());
                
                // Debug
//                g.setColour(juce::Colours::white);
//                g.drawRect(panel_bounds);
            }

            void resized() override
            {
                auto textPanel = juce::Rectangle<int>(title_area_bounds);
                mTitleLabel.setBounds(textPanel);
                
                auto panel = juce::Rectangle<int>(panel_bounds);
                mSyncToggle.setBounds(panel.removeFromTop(40));

                auto knobs = panel.removeFromTop(80);
                auto const rotaryWidth = static_cast<int>(knobs.getWidth() * 0.30);
                auto const spacing = static_cast<int>((knobs.getWidth() - (rotaryWidth * 3)) / 2.0);

                auto delaySliderBounds = knobs.removeFromLeft(rotaryWidth);
                mDelayTimeSlider.setBounds(delaySliderBounds);
                mSyncedDelaySlider.setBounds(delaySliderBounds);

                knobs.removeFromLeft(spacing);
                mWetDrySlider.setBounds(knobs.removeFromLeft(rotaryWidth));
                knobs.removeFromLeft(spacing);
                mFeedbackSlider.setBounds(knobs.removeFromLeft(rotaryWidth));
            }

        private:
            class SyncedDelayRotarySliderWithLabels : public RotarySliderWithLabels
            {
            public:
                using RotarySliderWithLabels::RotarySliderWithLabels;

                juce::String getDisplayString() const override
                {
                    auto const strVal = juce::String(syncedDelayDivisions[static_cast<int>(getValue())]);
                    return "1 / " + strVal;
                }
            };

            // juce::AudioProcessorValueTreeState::Listener
            void parameterChanged(const String& parameterID, float newValue) override
            {
                if(parameterID.equalsIgnoreCase("sync"))
                {
                    auto const synced = newValue >= 0.5f;
                    mDelayTimeSlider.setVisible(!synced);
                    mSyncedDelaySlider.setVisible(synced);

                    repaint();
                }
            }
            
            juce::Rectangle<int> const title_area_bounds{158, 32, 282, 45};
            juce::Rectangle<int> const panel_bounds{158, 102, 282, 125};
            
            juce::Label mTitleLabel;
            UI::PixelArt::SyncButton mSyncToggle;

            RotarySliderWithLabels mDelayTimeSlider;
            SyncedDelayRotarySliderWithLabels mSyncedDelaySlider;

            RotarySliderWithLabels mWetDrySlider;
            RotarySliderWithLabels mFeedbackSlider;

            juce::AudioProcessorValueTreeState::ButtonAttachment mSyncToggleAttachment;
            juce::AudioProcessorValueTreeState::SliderAttachment mDelayTimeAttachment;
            juce::AudioProcessorValueTreeState::SliderAttachment mSyncedDelayTimeAttachment;
            juce::AudioProcessorValueTreeState::SliderAttachment mWetDryAttachment;
            juce::AudioProcessorValueTreeState::SliderAttachment mFeedbackAttachment;
        };

        //==============================================================================
        AudioParameterBool* mSyncMode;
        AudioParameterFloat* mWetDryMix;
        AudioParameterFloat* mDelayTime;
        AudioParameterFloat* mFeedback;

        int mBlockSize;
        int mSampleRate;

        std::vector<std::unique_ptr<CircularBuffer<float>>> mDelayBuffers;
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleDelayProcessor)
    };
} // namespace OUS
