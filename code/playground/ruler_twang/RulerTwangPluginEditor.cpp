#include "RulerTwangPluginEditor.h"

using namespace OUS;

RulerTwangPluginProcessorEditor::RulerTwangPluginProcessorEditor(juce::AudioProcessor& owner, juce::AudioProcessorValueTreeState& state, std::function<void()> onTriggerClicked)
: juce::AudioProcessorEditor(owner)
, mState(state)
, mDecayTimeSlider("Decay Time", " ms")
, mYoungsModulusSlider("Youngs Modulus", " N/m^2")
, mRulerLengthSlider("Ruler Length", " mm")
, mRulerHeightSlider("Ruler Height", " mm")
, mRulerDensitySlider("Ruler Density", " kg/m^3")
, mDecayTimeAttachement(state, "decaytime", mDecayTimeSlider)
, mYoungsModulusAttachement(state, "youngsmodulus", mYoungsModulusSlider)
, mRulerLengthAttachement(state, "rulerlength", mRulerLengthSlider)
, mRulerHeightAttachement(state, "rulerheight", mRulerHeightSlider)
, mRulerDensityAttachement(state, "rulerdensity", mRulerDensitySlider)
, mFreeFrequencyField("Free frequency", "Hz", 2, false, 0.0)
, mClampedFrequencyField("Clamped frequency", "Hz", 2, false, 0.0)
{
    addAndMakeVisible(mTriggerButton);
    if(onTriggerClicked)
    {
        mTriggerButton.onClick = onTriggerClicked;
    }
    
    addAndMakeVisible(mDecayTimeSlider);
    mDecayTimeSlider.mLabels.add({0.0f, "10"});
    mDecayTimeSlider.mLabels.add({1.0f, "2000"});
    
    addAndMakeVisible(mYoungsModulusSlider);
    mYoungsModulusSlider.mLabels.add({0.0f, "1e9"});
    mYoungsModulusSlider.mLabels.add({1.0f, "2e10"});
    
    addAndMakeVisible(mRulerLengthSlider);
    mRulerLengthSlider.mLabels.add({0.0f, "100"});
    mRulerLengthSlider.mLabels.add({1.0f, "1000"});
    
    addAndMakeVisible(mRulerHeightSlider);
    mRulerHeightSlider.mLabels.add({0.0f, "1"});
    mRulerHeightSlider.mLabels.add({1.0f, "100"});
    
    addAndMakeVisible(mRulerDensitySlider);
    mRulerDensitySlider.mLabels.add({0.0f, "10"});
    mRulerDensitySlider.mLabels.add({1.0f, "1000"});
    
    addAndMakeVisible(mFreeFrequencyField);
    addAndMakeVisible(mClampedFrequencyField);
    
    setSize(600, 600);
}

RulerTwangPluginProcessorEditor::~RulerTwangPluginProcessorEditor()
{
}

void RulerTwangPluginProcessorEditor::setFreeFrequency(float frequency)
{
    mFreeFrequencyField.setValue(frequency, juce::NotificationType::dontSendNotification);
    repaint();
}

void RulerTwangPluginProcessorEditor::setClampedFrequency(float frequency)
{
    mClampedFrequencyField.setValue(frequency, juce::NotificationType::dontSendNotification);
    repaint();
}

void RulerTwangPluginProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void RulerTwangPluginProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);
    
    auto triggerBounds = bounds.removeFromTop(40);
    mTriggerButton.setBounds(triggerBounds.removeFromLeft(150));
    
    juce::Grid grid;
    grid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    grid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1))};
    
    grid.items = {  juce::GridItem(mDecayTimeSlider),
                    juce::GridItem(mYoungsModulusSlider),
                    juce::GridItem(mRulerLengthSlider),
                    juce::GridItem(mRulerHeightSlider),
                    juce::GridItem(mRulerDensitySlider)
    };
    
    auto gridBounds = bounds.removeFromTop(300);
    grid.performLayout(gridBounds);
    
    auto frequencyBounds = bounds.removeFromTop(40);
    mFreeFrequencyField.setBounds(frequencyBounds.removeFromLeft(100));
    frequencyBounds.removeFromLeft(20);
    mClampedFrequencyField.setBounds(frequencyBounds.removeFromLeft(100));
}

void RulerTwangPluginProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    
}
