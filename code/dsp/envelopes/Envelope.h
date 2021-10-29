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
    
    enum class EnvelopeType
    {
        trapezoidal,
        parabolic
    };
    
    struct Essence
    {
        virtual ~Essence() = default;
        
        float grainAmplitude {1.0};
    };
    
    Envelope(size_t sampleDuration);
    virtual ~Envelope() = default;
    
    virtual void init(size_t durationInSamples) = 0;
    virtual double synthesize() = 0;
    
protected:
    size_t mDuration; // length in samples
    size_t mPosition {0}; // position in env
};

class TrapezoidalEnvelope
: public Envelope
{
public:
    
    struct TrapezoidalEssence
    : Essence
    {
        size_t attackSamples;
        size_t releaseSamples;
    };
    
    TrapezoidalEnvelope(size_t sampleDuration, TrapezoidalEssence* essence);
    
    void init(size_t durationInSamples) override;
    double synthesize() override;
    
private:
    size_t mAttackSamples;
    size_t mReleaseSamples;
    
    float mGrainAmplitude {1.0f};
    float mPreviousAmplitude {0.0f};
};

class ParabolicEnvelope
: public Envelope
{
public:
    
    struct ParabolicEssence
    : Essence
    {
        
    };
    
    ParabolicEnvelope(size_t sampleDuration, ParabolicEssence* essence);
    
    void init(size_t durationInSamples) override;
    double synthesize() override;
    
private:
    float mAmplitude {0.0};
    float mRdur {0.0};
    float mRdur2 {0.0};
    float mSlope {0.0};
    float mCurve {0.0};
    
    float mGrainAmplitude {1.0f};
};
