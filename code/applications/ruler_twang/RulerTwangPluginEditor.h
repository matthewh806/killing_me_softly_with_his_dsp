#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    class RulerTwangPluginProcessorEditor
    : public juce::AudioProcessorEditor
    {
    public:
        RulerTwangPluginProcessorEditor(juce::AudioProcessor& owner,
                                        juce::AudioProcessorValueTreeState& state,
                                        std::function<void()> onTriggerClicked = nullptr,
                                        std::function<void(juce::String)> onRulerPresetChanged = nullptr,
                                        std::function<void()> onSavePreset = nullptr);
        
        ~RulerTwangPluginProcessorEditor() override;
        
        void setFreeFrequency(float frequency);
        void setClampedFrequency(float frequency);
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
    private:
        std::function<void(juce::String)> onRulerPresetChangedCallback = nullptr;
        
        juce::AudioProcessorValueTreeState& mState;
        
        juce::TextButton mTriggerButton { juce::String("Trigger") };
        RotarySliderWithLabels mDecayTimeSlider;
        RotarySliderWithLabels mYoungsModulusSlider;
        RotarySliderWithLabels mRulerLengthSlider;
        RotarySliderWithLabels mRulerHeightSlider;
        RotarySliderWithLabels mRulerDensitySlider;
        
        NumberFieldWithLabel mFreeFrequencyField;
        NumberFieldWithLabel mClampedFrequencyField;
        
        ComboBoxWithLabel mRulerPresetsCombobox {"Ruler Presets"} ;
        juce::TextButton mSavePresetButton { juce::String("Save Preset") };
        
        juce::AudioProcessorValueTreeState::SliderAttachment mDecayTimeAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mYoungsModulusAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mRulerLengthAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mRulerHeightAttachement;
        juce::AudioProcessorValueTreeState::SliderAttachment mRulerDensityAttachement;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RulerTwangPluginProcessorEditor)
    };
}
