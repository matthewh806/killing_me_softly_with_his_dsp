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
                           juce::Slider& slider) override;
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
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const;
    virtual juce::String getDisplayString() const;
    juce::String getParameterName() const;
    
    juce::Array<LabelPos> mLabels;

private:
    CustomLookAndFeel mLookAndFeel;
    
    juce::String mParamName;
    juce::String mSuffix;
};

class NumberFieldWithLabel : public juce::Component
{
public:
    NumberFieldWithLabel(juce::String const& paramName, juce::String const& unitSuffix = "", size_t const numberOfDecimals = 0, bool editable = true, double defaultValue = 0.0);
    NumberFieldWithLabel() = default;
    
    double getValue() const;
    void setValue(const double value, const juce::NotificationType notification);
    
    void setRange(juce::Range<double> const& range, juce::NotificationType notification);
    void setNumberOfDecimals(size_t numberOfDecimals);
    
    void resized() override;
    
    std::function<void(double)> onValueChanged = nullptr;
    
private:
    juce::Label mParamLabel;
    juce::Label mNumberField;
    
    size_t mNumberOfDecimals = 0;
    juce::Range<double> mRange = {std::numeric_limits<double>::min(), std::numeric_limits<double>::max()};
    juce::String mSuffix = juce::String();
};

