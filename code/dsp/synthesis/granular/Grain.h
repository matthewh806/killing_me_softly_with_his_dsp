#pragma once

#include <JuceHeader.h>
#include <functional>
#include "../../envelopes/Envelope.h"
#include "../../../core/ReferenceCountedBuffer.h"

class Grain
{
public:
    Grain();
    Grain(juce::AudioSampleBuffer* sampleBuffer);
    
    ~Grain();
    
    void init(size_t position, size_t duration, juce::AudioSampleBuffer* sampleBuffer);
    bool isGrainComplete() const;
    
    void synthesise(AudioBuffer<float>* buffer, int numSamples);
    
private:
    juce::Uuid mUuid;
    
    size_t mPosition {0}; // position in audio buffer
    size_t mDuration {0}; // grain duration in samples
    size_t mSampleCounter {0}; // keeps track of how many samples we've processed
    
    bool mComplete = false;
    
    juce::AudioSampleBuffer* mAudioSampleBuffer;
    Envelope mEnvelope;
};
