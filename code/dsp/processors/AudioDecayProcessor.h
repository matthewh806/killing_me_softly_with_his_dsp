#pragma once

#include "JuceHeader.h"

//==============================================================================
class AudioDecayProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioDecayProcessor();
    
    //==============================================================================
    
    void setQuantisationLevel(int bitDepth);
    void setDownsampleFactor(int downsampleFactor);
    void setWetDryMix(float mix);

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    //==============================================================================
    void prepareToPlay (double, int) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override            { return new juce::GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override                          { return true; }
    const String getName() const override                    { return "AudioDecay"; }
    bool acceptsMidi() const override                        { return false; }
    bool producesMidi() const override                       { return false; }
    double getTailLengthSeconds() const override             { return 0.0; }
    int getNumPrograms() override                            { return 1; }
    int getCurrentProgram() override                         { return 0; }
    void setCurrentProgram (int) override                    {}
    const String getProgramName (int) override               { return {}; }
    void changeProgramName (int, const String&) override     {}
    bool isVST2() const noexcept                             { return (wrapperType == wrapperType_VST); }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    int mBlockSize;
    int mSampleRate;
    
    std::atomic<float> mQuantisationLevel {1.0};
    std::atomic<int> mDownsampleFactor {1};
    std::atomic<float> mWetDryMix {1.0};

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDecayProcessor)
};

