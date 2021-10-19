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
        trapezoidal
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
    using Envelope::Envelope;
    
    void init(size_t durationInSamples) override;
    double synthesize() override;
    
private:
    size_t mAttackSamples;
    size_t mReleaseSamples;
    
    float mGrainAmplitude {1.0f};
    float mPreviousAmplitude {0.0f};
};
