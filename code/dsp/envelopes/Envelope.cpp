#include "Envelope.h"

Envelope::Envelope(size_t sampleDuration)
: mDuration(sampleDuration)
{
}

void TrapezoidalEnvelope::init(size_t durationInSamples)
{
    mDuration = durationInSamples;
    mAttackSamples = static_cast<size_t>(durationInSamples * 0.25);
    mReleaseSamples = static_cast<size_t>(durationInSamples * 0.25);
    mPreviousAmplitude = 0.0;
    mPosition = 0;
}

double TrapezoidalEnvelope::synthesize()
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
