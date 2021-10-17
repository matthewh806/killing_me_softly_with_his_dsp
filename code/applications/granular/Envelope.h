#pragma once

#include <iostream>
#include <functional>

/*
 This is just an implementation of  a trapezoidal envelope for now (Attack, Sustain, Release)
 Could be expanded to cover different types (e.g. parabolic)
 */
class Envelope
{
public:
    Envelope(size_t sampleDuration)
    : mDuration(sampleDuration)
    {
    }
    
    void init(size_t durationInSamples)
    {
        mDuration = durationInSamples;
        mAttackSamples = static_cast<size_t>(durationInSamples * 0.25);
        mReleaseSamples = static_cast<size_t>(durationInSamples * 0.25);
        mPreviousAmplitude = 0.0;
        mPosition = 0;
    }
    
    double synthesize()
    {
        if(mPosition > mDuration)
        {
            return 0.0;
        }
        
        // incmrement of 0 is the sustain portion
        auto amplitudeIncrement = 0.0f;
        auto const previousAmplitude = mPreviousAmplitude;
        
        if(mPosition < mAttackSamples)
        {
            amplitudeIncrement = mGrainAmplitude / static_cast<float>(mAttackSamples);
        }
        else if(mPosition >= mDuration - mReleaseSamples)
        {
            amplitudeIncrement = -mGrainAmplitude / static_cast<float>(mReleaseSamples);
        }
        
        
        auto const nextAmplitude = std::max(0.0f, std::min(1.0f, previousAmplitude + amplitudeIncrement));
        mPreviousAmplitude = nextAmplitude;
        mPosition++;
        
        return nextAmplitude;
    }
    
private:
    size_t mDuration; // length in samples
    size_t mPosition {0}; // position in env
    
    size_t mAttackSamples;
    size_t mReleaseSamples;
    
    float mGrainAmplitude {1.0f};
    float mPreviousAmplitude {0.0f};
};
