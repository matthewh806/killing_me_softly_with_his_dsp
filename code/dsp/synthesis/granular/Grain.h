#pragma once

#include <JuceHeader.h>
#include <functional>
#include "Source.h"
#include "../../envelopes/Envelope.h"
#include "../../../core/ReferenceCountedBuffer.h"

class Grain
{
public:
    Grain();
    ~Grain();
    
    void init(size_t position, size_t duration, juce::AudioSampleBuffer* sampleBuffer, Envelope::EnvelopeType envelopeType);
    bool isGrainComplete() const;
    
    void synthesise(AudioBuffer<float>* buffer, int numSamples);
    
private:
    juce::Uuid mUuid;
    
    size_t mDuration {0}; // grain duration in samples
    size_t mSampleCounter {0}; // keeps track of how many samples we've processed
    
    bool mComplete = false;
    
    std::unique_ptr<SampleSource> mSampleSource;
    std::unique_ptr<Envelope> mEnvelope;
};
