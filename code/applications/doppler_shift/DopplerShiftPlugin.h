#pragma once

#include <algorithm>
#include "../JuceLibraryCode/JuceHeader.h"
#include "RubberbandPitchShifter.h"

//==============================================================================
class DopplerShiftProcessor  : public AudioProcessor
{
public:

    //==============================================================================
    DopplerShiftProcessor()
        : AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                           .withOutput ("Output", AudioChannelSet::stereo()))
    {
        addParameter (mSourceFrequency = new AudioParameterFloat ("sourceFrequency", "SourceFrequency", 0.0f, 1000.0f, 220.0f));
        addParameter (mSourceVelocity = new AudioParameterFloat ("sourceVelocity", "SourceVelocity", 0.0f, 344.0f, 30.0f));
    }

    //==============================================================================
    void prepareToPlay (double samplesPerBlockExpected, int sampleRate) override
    {
        mPitchShifter = std::make_unique<RubberbandPitchShifter>(sampleRate, 2, samplesPerBlockExpected);
    }
    
    void releaseResources() override
    {
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiBuffer) override
    {
        mPitchShifter->process(buffer, buffer.getNumSamples());
    }

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override                        { return true;   }

    //==============================================================================
    const String getName() const override                  { return "DopplerShift PlugIn"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const String&) override   {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override
    {
        MemoryOutputStream (destData, true).writeFloat (*mSourceFrequency);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        mSourceFrequency->setValueNotifyingHost (MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
    }

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto& mainInLayout  = layouts.getChannelSet (true,  0);
        const auto& mainOutLayout = layouts.getChannelSet (false, 0);

        return (mainInLayout == mainOutLayout && (! mainInLayout.isDisabled()));
    }

private:
    //==============================================================================
    
    AudioParameterFloat* mSourceFrequency;
    AudioParameterFloat* mSourceVelocity;
    
    std::unique_ptr<RubberbandPitchShifter> mPitchShifter;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DopplerShiftProcessor)
};
