/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class PulsarAudioProcessor
: public AudioProcessor
, private juce::MidiInputCallback
, private juce::AsyncUpdater
{
public:
    //==============================================================================
    PulsarAudioProcessor();
    ~PulsarAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    void setMidiInput(juce::String const& identifier);
    void setMidiOutput(juce::String const& identifier);
    void sendNoteOnMessage(int noteNumber, float velocity);

private:
    //==============================================================================
    
    // juce::MidiInputCallback
    void handleIncomingMidiMessage (juce::MidiInput *source, juce::MidiMessage const& message) override;
    
    // juce::AsyncUpdater
    void handleAsyncUpdate() override;
    
    AudioDeviceManager mDeviceManager;
    
    std::unique_ptr<juce::MidiOutput> mMidiOutput;
    CriticalSection mMidiMonitorLock;
    Array<juce::MidiMessage> mIncomingMessages;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessor)
};
