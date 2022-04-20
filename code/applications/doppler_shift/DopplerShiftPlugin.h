#pragma once

#include <algorithm>
#include "../JuceLibraryCode/JuceHeader.h"

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
        addParameter (mSourceFrequency = new AudioParameterFloat ("sourceVelocity", "SourceVelocity", 0.0f, 344.0f, 30.0f));
    }

    //==============================================================================
    void prepareToPlay (double, int) override
    {
        
    }
    
    void releaseResources() override {}

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer&) override
    {
        auto* channelData = buffer.getReadPointer(0);
        for(auto i = 0; i < buffer.getNumSamples(); ++i)
        {
            if(mFifoIndex == mFFTSize)
            {
                std::fill(mFFTData.begin(), mFFTData.end(), 0.0f);
                std::copy(mFifo.begin(), mFifo.end(), mFFTData.begin());
                mFifoIndex = 0;
                mPerformFFT = true;
            }
            mFifo[mFifoIndex++] = channelData[i];
        }
        
        buffer.clear();
        // fill buffer for output if there is some available
        if(mOutputAvailable && mOutputIndex < mFFTSize)
        {
            auto outChannelData = buffer.getWritePointer(0);
            auto const samplesToRead = std::min(static_cast<size_t>(buffer.getNumSamples()), static_cast<size_t>(mFFTSize) - mOutputIndex);
            for(size_t i = 0; i < samplesToRead; ++i)
            {
                outChannelData[i] = mFFTData[i + mOutputIndex];
            }
            
            mOutputIndex += samplesToRead;
            if(mOutputIndex >= mFFTSize)
            {
                mOutputAvailable = false;
                mOutputIndex = 0;
            }
        }
        
        if(mPerformFFT)
        {
            mPerformFFT = false;
            mOutputAvailable = true;
            mFFT.performRealOnlyForwardTransform(mFFTData.data());
            mFFT.performRealOnlyInverseTransform(mFFTData.data());
        }
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
    static constexpr auto mFFTOrder = 10;
    static constexpr auto mFFTSize = 1 << mFFTOrder;
    
    AudioParameterFloat* mSourceFrequency;
    AudioParameterFloat* mSourceVelocity;
    
    juce::dsp::FFT mFFT {mFFTOrder};
    std::array<float, mFFTSize> mFifo;
    std::array<float, mFFTSize * 2> mFFTData;
    size_t mFifoIndex = 0;
    size_t mOutputIndex = 0;
    bool mPerformFFT = false;
    bool mOutputAvailable = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DopplerShiftProcessor)
};
