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
    mSequenceStrategy.setGrainDensity(grainsPerSecond);
}

void Scheduler::setPositionRandomness(double randomness)
{
    auto const length = mSampleBuffer->getNumSamples() - static_cast<int>(mGrainDuration.load());
    mPositionRandomness.store(static_cast<size_t>(length * randomness));
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
    auto grainPositionRandomness = mPositionRandomness.load();
    if(shouldSynthesise)
    {
        mGrainPool.synthesiseGrains(buffer, &mTempBuffer, numSamples);
    }
    
    while(mNextOnset < numSamples)
    {
        auto const nextInt = static_cast<size_t>(mRandom.nextInt(grainPositionRandomness == 0 ? 1 : static_cast<int>(grainPositionRandomness)));
        mGrainPool.create(nextInt, grainDuration, mSampleBuffer);
        mNextOnset += static_cast<size_t>(mSequenceStrategy.nextInteronset() * mSampleRate);
    }
    mNextOnset -= numSamples;
}

Scheduler::GrainPool::GrainPool()
{
}

void Scheduler::GrainPool::create(size_t position, size_t nextDuration, juce::AudioSampleBuffer* sampleBuffer)
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

size_t Scheduler::GrainPool::getNumberOfActiveGrains()
{
    return static_cast<size_t>(std::count_if(std::begin(mGrains), std::end(mGrains), [](auto const& grain)
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
            for(int s = 0; s < numSamples; ++s)
            {
                auto const gainVal = tmpBuffer->getSample(0, s);
                auto const unmixedVal = dest->getSample(0, s);
                dest->setSample(0, s, unmixedVal + gainVal * weight);
                dest->setSample(1, s, unmixedVal + gainVal * weight);
            }
        }
    }
}


