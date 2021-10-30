/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PulsarWorld.h"
#include "NoteStrategy.h"
#include "../../ui/CustomLookAndFeel.h"

#include <memory>
//==============================================================================
/**
*/

//! @todo: Add a free play mode that adds balls in a certain key at random. With random changes to rotation, gravity etc
//! @todo: Add ability to connect rotation, gravity etc with midi for things like lfos
class PulsarAudioProcessorEditor
: public juce::AudioProcessorEditor
, public juce::KeyListener
, public juce::Timer
, private juce::MidiInputCallback
, private juce::AsyncUpdater
{
public:
    PulsarAudioProcessorEditor (PulsarAudioProcessor&);
    ~PulsarAudioProcessorEditor() override;

    //==============================================================================

    void paint (Graphics&) override;
    void resized() override;
    
    void timerCallback() override
    {
        
    }
    
    // juce::keyPressed
    bool keyPressed(juce::KeyPress const& key) override;
    bool keyPressed (const KeyPress &key, Component *originatingComponent) override;
    
    void mouseUp (const MouseEvent& event) override;
    
    // juce::MidiInputCallback
    void handleIncomingMidiMessage (juce::MidiInput *source, juce::MidiMessage const& message) override;
    
    // juce::AsyncUpdater
    void handleAsyncUpdate() override;
    
    void sendNoteOnMessage(int noteNumber, float velocity);

private:
    void setMidiInput(juce::String const& identifier);
    void setMidiOutput(juce::String const& identifier);
    
    Physics::PulsarWorld mWorld {*this, { 0.0f, 0.0f, 4.0f, 4.0f }, {0.0f, 10.0f}};
    
    CriticalSection mMidiMonitorLock;
    Array<juce::MidiMessage> mIncomingMessages;
    
    AudioDeviceManager mDeviceManager;
    
    ComboBoxWithLabel mMidiInputDeviceList {"Midi in"};
    ComboBoxWithLabel mMidiInputChannelList {"Channel"};
    ComboBoxWithLabel mMidiOutputDeviceList {"Midi out"};
    ComboBoxWithLabel mMidiOutputChannelList {"Channel"};
    
    ComboBoxWithLabel mNoteStrategyList {"Scale"};
    ComboBoxWithLabel mNoteKey {"Key"};
    
    juce::Random mRandom;
    NoteStrategy mNoteStrategy;
    
    std::unique_ptr<juce::MidiOutput> mMidiOutput;
    int mMidiInputChannel;
    int mMidiOutputChannel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessorEditor)
};
