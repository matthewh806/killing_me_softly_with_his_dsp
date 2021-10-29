#include "Envelope.h"

Envelope::Envelope(size_t sampleDuration)
: mDuration(sampleDuration)
{
}

TrapezoidalEnvelope::TrapezoidalEnvelope(size_t sampleDuration, TrapezoidalEssence* essence)
: Envelope(sampleDuration)
, mGrainAmplitude(essence->grainAmplitude)
, mAttackSamples(essence->attackSamples)
, mReleaseSamples(essence->releaseSamples)
{
    mPreviousAmplitude = 0.0;
    mPosition = 0;
}

void TrapezoidalEnvelope::init(size_t durationInSamples)
{
    
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

ParabolicEnvelope::ParabolicEnvelope(size_t durationInSamples, ParabolicEssence* essence)
: Envelope(durationInSamples)
, mGrainAmplitude(essence->grainAmplitude)
{
    mAmplitude = 0.0f;
    mRdur = 1.0f / static_cast<float>(durationInSamples);
    mRdur2 = mRdur * mRdur;
    mSlope = 4.0f * mGrainAmplitude * (mRdur - mRdur2);
    mCurve = -8.0f * mGrainAmplitude * mRdur2;
}

void ParabolicEnvelope::init(size_t durationInSamples)
{
}

double ParabolicEnvelope::synthesize()
{
    mAmplitude = mAmplitude + mSlope;
    mSlope = mSlope + mCurve;
    
    return mAmplitude;
}
