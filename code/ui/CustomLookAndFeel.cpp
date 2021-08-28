#include "CustomLookAndFeel.h"

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider&)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    
    g.setColour(juce::Colour(97u, 18u, 167u));
    g.fillEllipse(bounds);
    
    g.setColour(juce::Colour(255u, 154u, 1u));
    g.drawEllipse(bounds, 1.0f);
    
    auto center = bounds.getCentre();
    juce::Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());
    
    Path p;
    p.addRectangle(r);
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    auto sliderAngleRad = juce::jmap(sliderPosProportional, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);
    p.applyTransform(juce::AffineTransform().rotated(sliderAngleRad, center.getX(), center.getY()));
    g.fillPath(p);
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    auto startAngle = juce::degreesToRadians(180.0f + 45.0f);
    auto endAngle = juce::degreesToRadians(180.0f - 45.0f) + juce::MathConstants<float>::twoPi;

    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    
    g.setColour(juce::Colours::red);
    g.drawRect(getLocalBounds());
    g.setColour(Colours::yellow);
    g.drawRect(sliderBounds);
    
    mLookAndFeel.drawRotarySlider(g,
                                  sliderBounds.getX(),
                                  sliderBounds.getY(),
                                  sliderBounds.getWidth(),
                                  sliderBounds.getHeight(),
                                  juce::jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                  startAngle,
                                  endAngle,
                                  *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto const bounds = getLocalBounds();
    auto const size = juce::jmin(bounds.getWidth(), bounds.getHeight()) - getTextHeight() * 2;
    
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
}

