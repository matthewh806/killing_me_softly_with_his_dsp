#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour (ComboBox::backgroundColourId, Colours::transparentBlack);
    setColour (ComboBox::buttonColourId, Colours::transparentBlack);
    setColour (ComboBox::arrowColourId, Colours::white);
    setColour (ComboBox::textColourId, Colours::black);
    setColour (ComboBox::outlineColourId, Colours::transparentBlack);
    
    setColour (PopupMenu::backgroundColourId, Colours::black);
    setColour (PopupMenu::highlightedBackgroundColourId, Colours::black);
    setColour (PopupMenu::highlightedTextColourId, Colours::white);
    setColour (PopupMenu::textColourId, Colours::white.withAlpha (0.8f));
    setColour (PopupMenu::headerTextColourId, Colours::white);
    
    setDefaultSansSerifTypefaceName("Avenir Next");
}

void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    
//    g.setColour(juce::Colour(97u, 18u, 167u));
//    g.fillEllipse(bounds);
//
//    g.setColour(juce::Colour(255u, 154u, 1u));
//    g.drawEllipse(bounds, 1.0f);
    
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
        g.setColour(juce::Colours::black);
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
    
    g.setColour(juce::Colours::black);
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
    return 12;
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

NumberField::NumberField (juce::String const& unitSuffix, size_t const numberOfDecimals, bool editable, double defaultValue)
: mNumberOfDecimals(numberOfDecimals)
, mSuffix(unitSuffix)
{
    setEditable(editable);
    setText(juce::String(defaultValue), juce::NotificationType::dontSendNotification);
}

void NumberField::paint(juce::Graphics& g)
{
    g.fillAll (findColour (Label::backgroundColourId));
    if (!isBeingEdited())
    {
        auto alpha = isEnabled() ? 1.0f : 0.5f;
        const Font font (getLookAndFeel().getLabelFont (*this));

        g.setColour (findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        juce::Rectangle<int> const textArea(getBorderSize().subtractedFrom(getLocalBounds()));
        g.drawFittedText (getValueAsText(), textArea, getJustificationType(),
                          jmax (1, (int) ((float) textArea.getHeight() / font.getHeight())),
                          getMinimumHorizontalScale());

        g.setColour (findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (isEnabled())
    {
        g.setColour (findColour (Label::outlineColourId));
    }

    g.drawRect (getLocalBounds());
}

void NumberField::setNumberOfDecimals(size_t numberOfDecimals)
{
    if(numberOfDecimals != mNumberOfDecimals)
    {
        mNumberOfDecimals = numberOfDecimals;
        
        if(mNumberOfDecimals == 0)
        {
            setKeyboardType(TextInputTarget::numericKeyboard);
        }
        else
        {
            setKeyboardType(TextInputTarget::decimalKeyboard);
        }
        
        repaint();
    }
}


juce::String NumberField::getValueAsText() const
{
    auto const value = getText().getDoubleValue();
    return juce::String(value, static_cast<int>(mNumberOfDecimals)) + juce::String(" ") + mSuffix;
}

NumberFieldWithLabel::NumberFieldWithLabel(juce::String const& paramName, juce::String const& unitSuffix, size_t const numberOfDecimals, bool editable, double defaultValue)
: mNumberField(unitSuffix, numberOfDecimals, editable, defaultValue)
{
    mParamLabel.setEditable(false);
    mParamLabel.setText(paramName, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mParamLabel);
    
    addAndMakeVisible(mNumberField);
    mNumberField.onEditorShow = [this]()
    {
        auto* ed = mNumberField.getCurrentTextEditor();
        ed->setInputRestrictions(10, "1234567890.");
    };
    
    mNumberField.onTextChange = [this]
    {
        setValue(getValue(), juce::NotificationType::sendNotification);
    };
}

double NumberFieldWithLabel::getValue() const
{
    return mNumberField.getText().getDoubleValue();
}

void NumberFieldWithLabel::setValue(const double value, const juce::NotificationType notification)
{
    auto const cappedValue = std::max(std::min(value, mRange.getEnd()), mRange.getStart());
    mNumberField.setText(juce::String(cappedValue, std::numeric_limits<double>::max_digits10), juce::NotificationType::dontSendNotification);
    if(onValueChanged != nullptr && notification == juce::sendNotification)
    {
        onValueChanged(mNumberField.getText().getDoubleValue());
    }
}

void NumberFieldWithLabel::setRange(juce::Range<double> const& range, juce::NotificationType notification)
{
    if(std::abs(range.getStart() - mRange.getStart()) >= std::numeric_limits<double>::epsilon()
       || std::abs(range.getEnd() - mRange.getEnd()) >= std::numeric_limits<double>::epsilon())
    {
        mRange = range;
        setValue(getValue(), notification);
    }
}

void NumberFieldWithLabel::setNumberOfDecimals(size_t numberOfDecimals)
{
    mNumberField.setNumberOfDecimals(numberOfDecimals);
}

void NumberFieldWithLabel::resized()
{
    auto bounds = getLocalBounds();
    mParamLabel.setBounds(bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.45)));
    bounds.removeFromLeft(static_cast<int>(2));
    mNumberField.setBounds(bounds);
}

ComboBoxWithLabel::ComboBoxWithLabel(juce::String const& paramName)
{
    mParamLabel.setEditable(false);
    mParamLabel.setText(paramName, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(mParamLabel);
    
    mParamLabel.setColour(Label::textColourId, juce::Colours::black);
    
//    auto curLookAndFeel = comboBox.getLookAndFeel();
//    curLookAndFeel.
    
    addAndMakeVisible(comboBox);
    
//    auto* rootPopup = comboBox.getRootMenu();
//    if(!rootPopup)
//    {
//        return;
//    }
//
//    rootPopup->setLookAndFeel(<#LookAndFeel *newLookAndFeel#>)
}

void ComboBoxWithLabel::resized()
{
    auto bounds = getLocalBounds();
    
    mParamLabel.setBounds(bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.45)));
    bounds.removeFromLeft(static_cast<int>(2));
    comboBox.setBounds(bounds);
}

void ComboBoxWithLabel::paint(juce::Graphics& g)
{
    auto const width = getWidth();
    auto const height = getHeight();
//    auto cornerSize = findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    Rectangle<int> boxBounds (0, 0, width, height);

//    g.setColour (findColour (ComboBox::backgroundColourId));
//    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

//    g.setColour (findColour (ComboBox::outlineColourId));
//    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    Rectangle<int> arrowZone (width - 30, 0, 20, height);
    Path path;
    path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
    path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
    path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

    g.setColour (findColour (ComboBox::arrowColourId).withAlpha ((isEnabled() ? 0.9f : 0.2f)));
    g.strokePath (path, PathStrokeType (2.0f));
}
