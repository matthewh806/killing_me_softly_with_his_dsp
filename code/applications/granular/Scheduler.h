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
        return mGrains.size();
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
        
        // cleanup old grains
        // todo: another thread...
        auto it = std::remove_if(mGrains.begin(), mGrains.end(), [](auto const& grain)
        {
            return grain->isGrainComplete();
        });
        mGrains.erase(it, mGrains.end());
        mTempBuffer.clear();
        
        if(shouldSynthesise && mGrains.size() > 0)
        {
            auto const weight = 1.0 / grainsPerUnitTime;
            for(auto& grain : mGrains)
            {
                grain->synthesise(&mTempBuffer, numSamples);
                
                for(size_t s = 0; s < numSamples; ++s)
                {
                    auto const gainVal = mTempBuffer.getSample(0, s);
                    auto const unmixedVal = buffer->getSample(0, s);
                    buffer->setSample(0, s, unmixedVal + gainVal);
                    buffer->setSample(1, s, unmixedVal + gainVal);
                }
            }
        }
        
        while(mNextOnset < numSamples)
        {
            auto const nextInt = mRandom.nextInt(mSampleBuffer->getNumSamples() - grainDuration);
            mGrains.push_back(std::make_unique<Grain>(nextInt, mSequenceStrategy.nextDuration(), mSampleBuffer));
            mNextOnset += mSequenceStrategy.nextInteronset() * mSampleRate;
        }
        mNextOnset -= numSamples;
    }
    
private:
    juce::Random mRandom;
    juce::AudioSampleBuffer* mSampleBuffer;
    
    SequenceStrategy mSequenceStrategy;
    
    std::atomic<size_t> mGrainDuration {0};
    std::atomic<double> mGrainsPerUnitTime {30.0};
    
    std::vector<std::unique_ptr<Grain>> mGrains;
    double mSampleRate {44100.0};
    
    AudioBuffer<float> mTempBuffer;
    
    int mNextOnset {0};
    
};
