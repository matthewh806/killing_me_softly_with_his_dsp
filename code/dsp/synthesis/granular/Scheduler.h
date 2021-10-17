#pragma once

#include "SequenceStrategy.h"
#include "Grain.h"
#include "../../envelopes/Envelope.h"

class Scheduler
{
public:
    Scheduler(juce::AudioSampleBuffer* sampleBuffer);
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    
    void setGrainDuration(size_t lengthInSamples);
    void setGrainDensity(double grainsPerSecond);
    
    size_t getNumberOfGrains();
    
    bool shouldSynthesise = false; // todo: remove
    void synthesise(AudioBuffer<float>* buffer, int numSamples);
    
private:
    class GrainPool
    {
    public:
        GrainPool();
        
        size_t getNumberOfActiveGrains();
        
        void create(size_t position, size_t nextDuration, juce::AudioSampleBuffer* sampleBuffer);
        void synthesiseGrains(AudioBuffer<float>* dest, AudioBuffer<float>* tmpBuffer, int numSamples);
        
    private:
        static const int POOL_SIZE = 200;
        Grain mGrains[POOL_SIZE];
    };
    
    juce::Random mRandom;
    juce::AudioSampleBuffer* mSampleBuffer;
    
    SequenceStrategy mSequenceStrategy;
    
    std::atomic<size_t> mGrainDuration {0};
    
    GrainPool mGrainPool;
    double mSampleRate {44100.0};
    
    AudioBuffer<float> mTempBuffer;
    
    int mNextOnset {0};
    
};
