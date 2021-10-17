#pragma once

class SequenceStrategy
{
public:
    SequenceStrategy()
    {
        
    }
    
    ~SequenceStrategy() = default;
    
    void setGrainDuration(size_t duration)
    {
        mDuration.store(duration);
    }
    
    void setGrainDensity(double grainDensity)
    {
        mGrainsPerUnitTime.store(grainDensity);
    }
    
    size_t nextDuration()
    {
        return mDuration.load();
    }
    
    float nextInteronset()
    {
        return -std::log(mRandom.nextFloat()) / mGrainsPerUnitTime.load();
    }

private:
    std::atomic<size_t> mDuration;
    std::atomic<double> mGrainsPerUnitTime {1.0};
    
    
    juce::Random mRandom;
};
