#pragma once

#include <JuceHeader.h>
#include <functional>
#include "Envelope.h"

class Grain
{
public:
    Grain(juce::AudioFormatReader& audioFormatReader, Envelope envelope)
    : mAudioReaderSource(audioFormatReader)
    , mEnvelope(envelope)
    {
        
    }
    
    ~Grain() = default;
    
    void activate()
    {
        mPosition = 0;
    }
    
    void synthesise(AudioBuffer<float>* buffer, int numSamples)
    {
        auto remaining = numSamples;
        auto outputBufPos = 0;
        while(--remaining > 0)
        {
            mAudioReaderSource.read(buffer, outputBufPos, 1, static_cast<int64>(mPosition), true, true);
            auto const val = buffer->getSample(0, outputBufPos) * static_cast<float>(mEnvelope.synthesize());
            buffer->setSample(0, outputBufPos, val);
            buffer->setSample(1, outputBufPos, val);
            
            mPosition += 1;
            outputBufPos += 1;
            
            if(mPosition == mAudioReaderSource.lengthInSamples - 1)
            {
                mPosition = 0;
            }
        }
        
        
    }
    
private:
    size_t mPosition {0};
    
    juce::AudioFormatReader& mAudioReaderSource;
    Envelope mEnvelope;
};
