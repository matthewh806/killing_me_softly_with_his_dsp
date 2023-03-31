#include "RulerTwangPluginEditor.h"

using namespace OUS;

RulerTwangPluginProcessorEditor::RulerTwangPluginProcessorEditor(juce::AudioProcessor& owner, juce::AudioProcessorValueTreeState& state, std::function<void()> onTriggerClicked, std::function<void(juce::String)> onRulerPresetChanged, std::function<void()> onSavePreset)
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
, onRulerPresetChangedCallback(onRulerPresetChanged)
{
    addAndMakeVisible(mTriggerButton);
    if(onTriggerClicked)
    {
        mTriggerButton.onClick = onTriggerClicked;
    }
    
    addAndMakeVisible(mRulerPresetsCombobox);
    mRulerPresetsCombobox.comboBox.addItem("Initial", 1);
    mRulerPresetsCombobox.comboBox.addItem("Wooden Ruler", 2);
    if(onRulerPresetChanged)
    {
        mRulerPresetsCombobox.comboBox.onChange = [&]()
        {
            auto const idx = mRulerPresetsCombobox.comboBox.getSelectedItemIndex();
            auto const presetName = mRulerPresetsCombobox.comboBox.getItemText(idx);
            onRulerPresetChangedCallback(presetName);
        };
    }
    
    addAndMakeVisible(mSavePresetButton);
    mSavePresetButton.onClick = onSavePreset;
    
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
    
    auto functionsGridBounds = bounds.removeFromTop(40);
    juce::Grid functionsGrid;
    functionsGrid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    functionsGrid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(3)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    functionsGrid.items = { juce::GridItem(mTriggerButton), juce::GridItem(mRulerPresetsCombobox), juce::GridItem(mSavePresetButton) };
    functionsGrid.performLayout(functionsGridBounds);
    
    bounds.removeFromTop(40);
    
    juce::Grid rulerPropertiesGrid;
    rulerPropertiesGrid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    rulerPropertiesGrid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1))};
    
    rulerPropertiesGrid.items = {  juce::GridItem(mDecayTimeSlider),
                    juce::GridItem(mYoungsModulusSlider),
                    juce::GridItem(mRulerLengthSlider),
                    juce::GridItem(mRulerHeightSlider),
                    juce::GridItem(mRulerDensitySlider)
    };
    
    auto rulerPropertiesGridBounds = bounds.removeFromTop(300);
    rulerPropertiesGrid.performLayout(rulerPropertiesGridBounds);
    
    auto frequencyBounds = bounds.removeFromTop(40);
    mFreeFrequencyField.setBounds(frequencyBounds.removeFromLeft(200));
    frequencyBounds.removeFromLeft(30);
    mClampedFrequencyField.setBounds(frequencyBounds.removeFromLeft(200));
}
