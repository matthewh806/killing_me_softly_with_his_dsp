#include "CustomLookAndFeel.h"

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    
    g.setColour(juce::Colour(97u, 18u, 167u));
    g.fillEllipse(bounds);
    
    g.setColour(juce::Colour(255u, 154u, 1u));
    g.drawEllipse(bounds, 1.0f);
    
    if(auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        juce::Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5f);
        
        Path p;
        p.addRoundedRectangle(r, 2.0f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngleRad = juce::jmap(sliderPosProportional, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);
        p.applyTransform(juce::AffineTransform().rotated(sliderAngleRad, center.getX(), center.getY()));
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto const text = rswl->getDisplayString();
        auto const strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(juce::Colours::black);
        g.fillRect(r);
        
        g.setColour(juce::Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    auto startAngle = juce::degreesToRadians(180.0f + 45.0f);
    auto endAngle = juce::degreesToRadians(180.0f - 45.0f) + juce::MathConstants<float>::twoPi;

    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    
    // For debugging purposes:
//    g.setColour(juce::Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    {
        juce::Rectangle<float> r;
        auto sliderLabel = mParamName;
        r.setSize(g.getCurrentFont().getStringWidth(sliderLabel), getTextHeight());
        r.setCentre(getLocalBounds().getCentre().getX(), 0);
        r.setY(2);
        g.setColour(juce::Colour(0u, 172u, 1u));
        g.setFont(getTextHeight());
        g.drawFittedText(sliderLabel, r.toNearestInt(), juce::Justification::centred, 1);
        
        mLookAndFeel.drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      static_cast<float>(juce::jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0)),
                                      startAngle,
                                      endAngle,
                                      *this);
    }
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(juce::Colour(0u, 172u, 1u));
    g.setFont(getTextHeight());
    
    auto const numChoices = mLabels.size();
    for(int i = 0; i < numChoices; ++i)
    {
        auto const pos = mLabels[i].pos;
        jassert(0.0f <= pos);
        jassert(pos <= 1.0f);
        
        auto ang = jmap(pos, 0.0f, 1.0f, startAngle, endAngle);
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1.0f, ang);
        
        juce::Rectangle<float> r;
        auto str = mLabels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto const bounds = getLocalBounds();
    auto const size = juce::jmin(bounds.getWidth(), bounds.getHeight()) - getTextHeight() * 2;
    
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(getTextHeight() + 2);
    
    return r;
}

int RotarySliderWithLabels::getTextHeight() const
{
    return 14;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    auto str = juce::String(getValue(), 2);
    if(mSuffix.isNotEmpty())
    {
        str << mSuffix;
    }
    
    return str;
}

juce::String RotarySliderWithLabels::getParameterName() const
{
    return mParamName;
}

NumberFieldWithLabel::NumberFieldWithLabel(juce::String const& paramName, juce::String const& unitSuffix, bool editable, double defaultValue)
{
    mParamLabel.setEditable(false);
    mParamLabel.setText(paramName, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mParamLabel);
    
    mNumberField.setEditable(editable);
    addAndMakeVisible(mNumberField);
    mNumberField.setText(juce::String(defaultValue), juce::NotificationType::dontSendNotification);
    
    mNumberField.onEditorShow = [this]()
    {
        auto* ed = mNumberField.getCurrentTextEditor();
        ed->setInputRestrictions(3, "1234567890");
    };
    
    mNumberField.onTextChange = [this]
    {
        if(onValueChanged != nullptr)
        {
            onValueChanged(mNumberField.getText().getDoubleValue());
        }
    };
}

void NumberFieldWithLabel::setValue(const double value, const juce::NotificationType notification)
{
    mNumberField.setText(juce::String(value), notification);
}

void NumberFieldWithLabel::resized()
{
    auto bounds = getLocalBounds();
    
    mParamLabel.setBounds(bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.45)));
    bounds.removeFromLeft(static_cast<int>(2));
    mNumberField.setBounds(bounds);
}
