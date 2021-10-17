#include "Scheduler.h"

Scheduler::Scheduler(juce::AudioSampleBuffer* sampleBuffer)
: mSampleBuffer(sampleBuffer)
, mGrainDuration(std::min(static_cast<size_t>(4096*2), static_cast<size_t>(sampleBuffer->getNumSamples())))
{
    
}

void Scheduler::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    mSampleRate = sampleRate;
    mTempBuffer.setSize(2, samplesPerBlockExpected);
}

void Scheduler::setGrainDuration(size_t lengthInSamples)
{
    mGrainDuration.store(lengthInSamples);
    mSequenceStrategy.setGrainDuration(lengthInSamples);
}

void Scheduler::setGrainDensity(double grainsPerSecond)
{
    mGrainsPerUnitTime.store(grainsPerSecond);
    mSequenceStrategy.setGrainDensity(grainsPerSecond);
}

size_t Scheduler::getNumberOfGrains()
{
    return mGrainPool.getNumberOfActiveGrains();
}

bool shouldSynthesise = false; // todo: remove
void Scheduler::synthesise(AudioBuffer<float>* buffer, int numSamples)
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

Scheduler::GrainPool::GrainPool()
{
};

void Scheduler::GrainPool::create(int position, size_t nextDuration, juce::AudioSampleBuffer* sampleBuffer)
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

int Scheduler::GrainPool::getNumberOfActiveGrains()
{
    return static_cast<int>(std::count_if(std::begin(mGrains), std::end(mGrains), [](auto const& grain)
    {
        return !grain.isGrainComplete();
    }));
}

void Scheduler::GrainPool::synthesiseGrains(AudioBuffer<float>* dest, AudioBuffer<float>* tmpBuffer, int numSamples)
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


