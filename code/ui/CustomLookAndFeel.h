#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;
};

class RotarySliderWithLabels : public juce::Slider
{
public:
    RotarySliderWithLabels(juce::String const& paramName, juce::String const& unitSuffix)
    : juce::Slider (juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
    , mParamName(paramName)
    , mSuffix(unitSuffix)
    {
        setLookAndFeel(&mLookAndFeel);
    }
    
    ~RotarySliderWithLabels() override
    {
        setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    juce::String getDisplayString() const;

private:
    CustomLookAndFeel mLookAndFeel;
    
    juce::String mParamName;
    juce::String mSuffix;
};
