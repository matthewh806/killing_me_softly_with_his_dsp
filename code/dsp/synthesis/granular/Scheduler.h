#pragma once

#include "SequenceStrategy.h"
#include "Grain.h"
#include "../../envelopes/Envelope.h"

class Scheduler
{
public:
    Scheduler();
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate);
    
    void setGrainDuration(size_t lengthInSamples);
    void setGrainDensity(double grainsPerSecond);
    void setEnvelopeType(Envelope::EnvelopeType envelopeType);
    
    void setSourceEssence(std::unique_ptr<Source::Essence> essence);
    Source::Essence* getSourceEssence();
    
    // todo: add set position
    // this is between 0.0 and 1.0
    void setPositionRandomness(double positionRandomness);
    
    size_t getNumberOfGrains();
    
    bool shouldSynthesise = false; // todo: remove
    void synthesise(AudioBuffer<float>* buffer, int numSamples);
    
private:
    class GrainPool
    {
    public:
        GrainPool();
        
        size_t getNumberOfActiveGrains();
        
        void create(size_t nextDuration, Source::Essence* sourceEssence, Envelope::EnvelopeType envelopeType);
        void synthesiseGrains(AudioBuffer<float>* dest, AudioBuffer<float>* tmpBuffer, int numSamples);
        
    private:
        static const int POOL_SIZE = 200;
        Grain mGrains[POOL_SIZE];
    };
    
    juce::Random mRandom;
    
    SequenceStrategy mSequenceStrategy;
    
    std::atomic<size_t> mGrainDuration {0};
    std::atomic<size_t> mPositionRandomness {0}; // the position randomness in terms of samples
    
    std::unique_ptr<Source::Essence> mSourceEssence {nullptr};
    Envelope::EnvelopeType mEnvelopeType { Envelope::EnvelopeType::trapezoidal };
    
    GrainPool mGrainPool;
    double mSampleRate {44100.0};
    
    AudioBuffer<float> mTempBuffer;
    
    int mNextOnset {0};
    
};
