#include "Scheduler.h"

Scheduler::Scheduler()
: mGrainDuration(4096*2)
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
    auto essence = dynamic_cast<SampleSource::SampleEssence*>(mSourceEssence.get());
    if(essence == nullptr)
    {
        std::cout << "Could not cast Schedulers Essence to the expected type: SampleSource::SampleEssence\n";
        return;
    }
    
    auto const length = essence->audioSampleBuffer->getNumSamples() - static_cast<int>(mGrainDuration.load());
    mPositionRandomness.store(static_cast<size_t>(length * randomness));
}

void Scheduler::setSourceEssence(std::unique_ptr<Source::Essence> essence)
{
    mSourceEssence = std::move(essence);
}

Source::Essence* Scheduler::getSourceEssence()
{
    return mSourceEssence.get();
}

void Scheduler::setEnvelopeEssence(std::unique_ptr<Envelope::Essence> essence)
{
    mEnvelopeEssence = std::move(essence);
}

Envelope::Essence* Scheduler::getEnvelopeEssence()
{
    return mEnvelopeEssence.get();
}


size_t Scheduler::getNumberOfGrains()
{
    return mGrainPool.getNumberOfActiveGrains();
}

bool shouldSynthesise = false; // todo: remove
void Scheduler::synthesise(AudioBuffer<float>* buffer, int numSamples)
{
    auto grainDuration = mGrainDuration.load();
    auto grainPositionRandomness = mPositionRandomness.load();
    
    if(shouldSynthesise)
    {
        mGrainPool.synthesiseGrains(buffer, &mTempBuffer, numSamples);
    }
    
    while(mNextOnset < numSamples)
    {
        auto essence = dynamic_cast<SampleSource::SampleEssence*>(mSourceEssence.get());
        if(essence != nullptr)
        {
            essence->position = static_cast<size_t>(mRandom.nextInt(grainPositionRandomness == 0 ? 1 : static_cast<int>(grainPositionRandomness)));
        }
        mGrainPool.create(grainDuration, mSourceEssence.get(), mEnvelopeEssence.get());
        mNextOnset += static_cast<size_t>(mSequenceStrategy.nextInteronset() * mSampleRate);
    }
    mNextOnset -= numSamples;
}

Scheduler::GrainPool::GrainPool()
{
}

void Scheduler::GrainPool::create(size_t nextDuration, Source::Essence* sourceEssence, Envelope::Essence* envelopeEssence)
{
    if(sourceEssence == nullptr)
    {
        std::cerr << "Error creating grain: Source Essence is a nullptr\n";
        return;
    }
    
    for(size_t i = 0; i < POOL_SIZE; ++i)
    {
        if(mGrains[i].isGrainComplete())
        {
            mGrains[i].init(nextDuration, sourceEssence, envelopeEssence);
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


