#pragma once

#include "SequenceStrategy.h"
#include "Grain.h"
#include "Envelope.h"

class Scheduler
{
public:
    Scheduler(juce::AudioFormatReader& audioFormatReaderSource)
    : mAudioReaderSource(audioFormatReaderSource)
    , mGrain(audioFormatReaderSource, mEnvelope)
    {
        
    }
    
    bool shouldSynthesise = false; // todo: remove
    void synthesise(AudioBuffer<float>* buffer, int numSamples)
    {
        if(shouldSynthesise)
        {
            return mGrain.synthesise(buffer, numSamples);
        }
    }
    
private:
    juce::AudioFormatReader& mAudioReaderSource;
    
    SequenceStrategy mSequenceStrategy;
    Grain mGrain; // just one for now!
    Envelope mEnvelope;
};
