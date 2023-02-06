#include "SequenceStrategy.h"

using namespace OUS;

SequenceStrategy::SequenceStrategy()
{
    
}

void SequenceStrategy::setGrainDuration(size_t duration)
{
    mDuration.store(duration);
}

void SequenceStrategy::setGrainDensity(double grainDensity)
{
    mGrainsPerUnitTime.store(grainDensity);
}

size_t SequenceStrategy::nextDuration()
{
    return mDuration.load();
}

float SequenceStrategy::nextInteronset()
{
    return -std::log(mRandom.nextFloat()) / static_cast<float>(mGrainsPerUnitTime.load());
}

