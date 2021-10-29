#pragma once

#include "SequenceStrategy.h"
#include "Grain.h"
#include "../../envelopes/Envelope.h"

class Scheduler
{
public:
    Scheduler(juce::AudioSampleBuffer* sampleBuffer, Source::SourceType sourceType);
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    
    void setGrainDuration(size_t lengthInSamples);
    void setGrainDensity(double grainsPerSecond);
    void setEnvelopeType(Envelope::EnvelopeType envelopeType);
    
    // todo: add set position
    // this is between 0.0 and 1.0
    void setPositionRandomness(double positionRandomness);
    
    void setOscillatorFrequency(double frequency);
    
    size_t getNumberOfGrains();
    
    bool shouldSynthesise = false; // todo: remove
    void synthesise(AudioBuffer<float>* buffer, int numSamples);
    
private:
    class GrainPool
    {
    public:
        GrainPool();
        
        size_t getNumberOfActiveGrains();
        
        void create(size_t position, size_t nextDuration, double frequency, Source::SourceType sourceType, Envelope::EnvelopeType envelopeType, juce::AudioSampleBuffer* sampleBuffer);
        void synthesiseGrains(AudioBuffer<float>* dest, AudioBuffer<float>* tmpBuffer, int numSamples);
        
    private:
        static const int POOL_SIZE = 200;
        Grain mGrains[POOL_SIZE];
    };
    
    juce::Random mRandom;
    juce::AudioSampleBuffer* mSampleBuffer;
    
    SequenceStrategy mSequenceStrategy;
    
    std::atomic<size_t> mGrainDuration {0};
    std::atomic<size_t> mPositionRandomness {0}; // the position randomness in terms of samples
    
    double mOscillatorFrequency = 220.0;
    
    Source::SourceType mSourceType;
    Envelope::EnvelopeType mEnvelopeType { Envelope::EnvelopeType::trapezoidal };
    
    GrainPool mGrainPool;
    double mSampleRate {44100.0};
    
    AudioBuffer<float> mTempBuffer;
    
    int mNextOnset {0};
    
};
