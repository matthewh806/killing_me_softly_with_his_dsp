#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    class RulerTwangPluginProcessorEditor
    : public juce::AudioProcessorEditor
    , private juce::AudioProcessorValueTreeState::Listener
    {
    public:
        RulerTwangPluginProcessorEditor(juce::AudioProcessor& owner, juce::AudioProcessorValueTreeState& state, std::function<void()> onTriggerClicked = nullptr);
        ~RulerTwangPluginProcessorEditor() override;
        
        void setFreeFrequency(float frequency);
        void setClampedFrequency(float frequency);
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
        void parameterChanged (const String& parameterID, float newValue) override;
        
    private:
        
        juce::AudioProcessorValueTreeState& mState;
        
        juce::TextButton mTriggerButton { juce::String("Trigger") };
        RotarySliderWithLabels mDecayTimeSlider;
        RotarySliderWithLabels mYoungsModulusSlider;
        RotarySliderWithLabels mRulerLengthSlider;
        RotarySliderWithLabels mRulerHeightSlider;
        RotarySliderWithLabels mRulerDensitySlider;
        
        NumberFieldWithLabel mFreeFrequencyField;
        NumberFieldWithLabel mClampedFrequencyField;
        
        juce::AudioProcessorValueTreeState::SliderAttachment mDecayTimeAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mYoungsModulusAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mRulerLengthAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mRulerHeightAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mRulerDensityAttachement;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RulerTwangPluginProcessorEditor)
    };
}
