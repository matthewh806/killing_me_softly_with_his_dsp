#pragma once

#include <JuceHeader.h>
#include <functional>
#include "Envelope.h"
#include "../../core/ReferenceCountedBuffer.h"

class Grain
{
public:
    Grain(size_t position, size_t duration, juce::AudioSampleBuffer* sampleBuffer)
    : mPosition(position)
    , mDuration(duration)
    , mAudioSampleBuffer(sampleBuffer)
    , mEnvelope(duration)
    {
        //std::cout << "Created grain: id: " << mUuid.toDashedString() << ", pos: " << mPosition << ", duration: " << mDuration << "\n";
    }
    
    ~Grain()
    {
        //std::cout << "End of grain: id: " << mUuid.toDashedString() << "\n";
    }
    
    void activate()
    {
        mPosition = 0;
    }
    
    bool isGrainComplete() const
    {
        return mComplete;
    }
    
    void synthesise(AudioBuffer<float>* buffer, int numSamples)
    {
        if(mAudioSampleBuffer == nullptr)
        {
            // todo: uh oh...
            buffer->clear();
            return;
        }
        
        auto remaining = numSamples;
        auto outputBufPos = 0;
        while(--remaining > 0 && !mComplete)
        {
            auto const val =  mAudioSampleBuffer->getSample(0, static_cast<int64>(mPosition)) * static_cast<float>(mEnvelope.synthesize());
            buffer->setSample(0, outputBufPos, val);
            buffer->setSample(1, outputBufPos, val);
            
            mPosition += 1;
            outputBufPos += 1;
            
            if(mPosition == mAudioSampleBuffer->getNumSamples() - 1)
            {
                mComplete = true;
            }
        }
        
        
    }
    
private:
    juce::Uuid mUuid;
    
    size_t mPosition {0};
    size_t mDuration {0}; // grain duration in samples
    
    bool mComplete = false;
    
    juce::AudioSampleBuffer* mAudioSampleBuffer;
    Envelope mEnvelope;
};
