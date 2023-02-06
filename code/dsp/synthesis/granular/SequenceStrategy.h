#pragma once

#include <JuceHeader.h>

namespace OUS
{
    class SequenceStrategy
    {
    public:
        SequenceStrategy();
        ~SequenceStrategy() = default;
        
        void setGrainDuration(size_t duration);
        void setGrainDensity(double grainDensity);
        
        size_t nextDuration();
        float nextInteronset();

    private:
        std::atomic<size_t> mDuration;
        std::atomic<double> mGrainsPerUnitTime {1.0};
        
        juce::Random mRandom;
    };
}
