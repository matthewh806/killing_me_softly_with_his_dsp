#pragma once

#include "SequenceStrategy.h"
#include "Grain.h"
#include "Envelope.h"

class Scheduler
{
public:
    Scheduler(juce::AudioSampleBuffer* sampleBuffer)
    : mSampleBuffer(sampleBuffer)
    , mGrainDuration(std::min(static_cast<size_t>(4096*2), static_cast<size_t>(sampleBuffer->getNumSamples())))
    {
        
    }
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate)
    {
        mSampleRate = sampleRate;
        mTempBuffer.setSize(2, samplesPerBlockExpected);
    }
    
    void setGrainDuration(size_t lengthInSamples)
    {
        mGrainDuration.store(lengthInSamples);
        mSequenceStrategy.setGrainDuration(lengthInSamples);
    }
    
    void setGrainDensity(double grainsPerSecond)
    {
        mGrainsPerUnitTime.store(grainsPerSecond);
        mSequenceStrategy.setGrainDensity(grainsPerSecond);
    }
    
    size_t getNumberOfGrains()
    {
        return mGrainPool.getNumberOfActiveGrains();
    }
    
    bool shouldSynthesise = false; // todo: remove
    void synthesise(AudioBuffer<float>* buffer, int numSamples)
    {
        if(mSampleBuffer == nullptr)
        {
            buffer->clear();
            return;
        }
        
        auto grainDuration = mGrainDuration.load();
        auto grainsPerUnitTime = mGrainsPerUnitTime.load();
        
        if(shouldSynthesise)
        {
            mGrainPool.synthesiseGrains(buffer, &mTempBuffer, numSamples);
        }
        
        while(mNextOnset < numSamples)
        {
            auto const nextInt = mRandom.nextInt(mSampleBuffer->getNumSamples() - grainDuration);
            mGrainPool.create(nextInt, grainDuration, mSampleBuffer);
            mNextOnset += mSequenceStrategy.nextInteronset() * mSampleRate;
        }
        mNextOnset -= numSamples;
    }
    
private:
    class GrainPool
    {
    public:
        GrainPool()
        {
        };
        
        void create(int position, size_t nextDuration, juce::AudioSampleBuffer* sampleBuffer)
        {
            for(size_t i = 0; i < POOL_SIZE; ++i)
            {
                if(mGrains[i].isGrainComplete())
                {
                    mGrains[i].init(position, nextDuration, sampleBuffer);
                    return;
                }
            }
        }
        
        int getNumberOfActiveGrains()
        {
            return static_cast<int>(std::count_if(std::begin(mGrains), std::end(mGrains), [](auto const& grain)
            {
                return !grain.isGrainComplete();
            }));
        }
        
        void synthesiseGrains(AudioBuffer<float>* dest, AudioBuffer<float>* tmpBuffer, int numSamples)
        {
            auto const activeGrains = getNumberOfActiveGrains();
            auto const weight = 1.0f / static_cast<float>(activeGrains);
            
            for(auto& grain : mGrains)
            {
                if(!grain.isGrainComplete())
                {
                    grain.synthesise(tmpBuffer, numSamples);
                    for(size_t s = 0; s < numSamples; ++s)
                    {
                        auto const gainVal = tmpBuffer->getSample(0, s);
                        auto const unmixedVal = dest->getSample(0, s);
                        dest->setSample(0, s, unmixedVal + gainVal * weight);
                        dest->setSample(1, s, unmixedVal + gainVal * weight);
                    }
                }
            }
        }
        
    private:
        static const int POOL_SIZE = 200;
        Grain mGrains[POOL_SIZE];
    };
    
    juce::Random mRandom;
    juce::AudioSampleBuffer* mSampleBuffer;
    
    SequenceStrategy mSequenceStrategy;
    
    std::atomic<size_t> mGrainDuration {0};
    std::atomic<double> mGrainsPerUnitTime {30.0};
    
    GrainPool mGrainPool;
    double mSampleRate {44100.0};
    
    AudioBuffer<float> mTempBuffer;
    
    int mNextOnset {0};
    
};
