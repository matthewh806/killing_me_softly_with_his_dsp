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
    Envelope(size_t sampleDuration);
    
    void init(size_t durationInSamples);
    double synthesize();
    
private:
    size_t mDuration; // length in samples
    size_t mPosition {0}; // position in env
    
    size_t mAttackSamples;
    size_t mReleaseSamples;
    
    float mGrainAmplitude {1.0f};
    float mPreviousAmplitude {0.0f};
};
