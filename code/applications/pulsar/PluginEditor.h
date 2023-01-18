/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "NoteStrategy.h"
#include "../../ui/CustomLookAndFeel.h"

#include <memory>

class InformationScreen
: public juce::Component
{
public:
    InformationScreen()
    {
        addAndMakeVisible(mControlsLabel);
        mControlsLabel.setText(text, juce::NotificationType::dontSendNotification);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        mControlsLabel.setBounds(bounds.reduced(20, 20));
    }
    
private:
    const char *text =
      "Controls:\n"
      " r: increment rotation speed\n"
      " i: increase the polygon edge separation\n"
      " d: decrease the polygon edge separation\n"
      " n [3-8]: create polygon with n sides\n"
      " Set Gravity with the number field \n"
      " Set scale / key with the drop down fields \n";
    
    juce::Label mControlsLabel;
};

//! @todo: note hardcoded. this could be based on the length of a mouse down for e.g. (or rand)
#define NOTE_OFF_TIME_MS 100

//! @todo: With random changes to rotation, gravity etc
//! @todo: Add ability to connect rotation, gravity etc with midi for things like lfos
class PulsarAudioProcessorEditor
: public juce::AudioProcessorEditor
, public juce::KeyListener
{
public:
    PulsarAudioProcessorEditor (PulsarAudioProcessor& processor, AudioDeviceManager& deviceManager);
    ~PulsarAudioProcessorEditor() override;

    //==============================================================================

    void paint (Graphics&) override;
    void resized() override;
    
    // juce::keyPressed
    bool keyPressed(juce::KeyPress const& key) override;
    bool keyPressed (const KeyPress &key, Component *originatingComponent) override;
    
    void mouseUp (const MouseEvent& event) override;

private:
    void showInformationScreen();
    
    AudioDeviceManager& mDeviceManager;
    
    ComboBoxWithLabel mMidiInputDeviceList {"Midi in"};
    ComboBoxWithLabel mMidiInputChannelList {"Channel"};
    ComboBoxWithLabel mMidiOutputDeviceList {"Midi out"};
    ComboBoxWithLabel mMidiOutputChannelList {"Channel"};
    
    NumberFieldWithLabel mGravityField { "Gravity", "N", 1, true };
    
    ComboBoxWithLabel mNoteStrategyList {"Scale"};
    ComboBoxWithLabel mNoteKey {"Key"};
    
    InformationScreen mInformationScreen;
    
    juce::Random mRandom;
    NoteStrategy mNoteStrategy;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessorEditor)
};
