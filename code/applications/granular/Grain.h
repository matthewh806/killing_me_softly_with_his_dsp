#pragma once

#include <JuceHeader.h>
#include <functional>
#include "Envelope.h"
#include "../../core/ReferenceCountedBuffer.h"

class Grain
{
public:
    Grain()
    : mPosition(0)
    , mDuration(0)
    , mComplete(true)
    , mAudioSampleBuffer(nullptr)
    , mEnvelope(0)
    {
        
    }
    
    Grain(juce::AudioSampleBuffer* sampleBuffer)
    : mPosition(0)
    , mDuration(0)
    , mComplete(true)
    , mAudioSampleBuffer(sampleBuffer)
    , mEnvelope(0)
    {
        //std::cout << "Created grain: id: " << mUuid.toDashedString() << ", pos: " << mPosition << ", duration: " << mDuration << "\n";
    }
    
    ~Grain()
    {
        //std::cout << "End of grain: id: " << mUuid.toDashedString() << "\n";
    }
    
    void init(size_t position, size_t duration, juce::AudioSampleBuffer* sampleBuffer)
    {
        mPosition = position;
        mDuration = duration;
        mAudioSampleBuffer = sampleBuffer;
        
        mSampleCounter = 0;
        mEnvelope.init(duration);
        mComplete = false;
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
            mSampleCounter += 1;
            
            if(mSampleCounter == mDuration)
            {
                mComplete = true;
            }
        }
        
        
    }
    
private:
    juce::Uuid mUuid;
    
    size_t mPosition {0}; // position in audio buffer
    size_t mDuration {0}; // grain duration in samples
    size_t mSampleCounter {0}; // keeps track of how many samples we've processed
    
    bool mComplete = false;
    
    juce::AudioSampleBuffer* mAudioSampleBuffer;
    Envelope mEnvelope;
};
