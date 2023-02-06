#pragma once

#include <iostream>
#include "JuceHeader.h"

namespace OUS
{
    class Source
    {
    public:
        enum class SourceType
        {
            sample,
            synthetic
        };
        
        struct Essence
        {
            virtual ~Essence() = default;
        };
        
        virtual ~Source() = default;
        
        virtual size_t getLastPosition() const;
        virtual double synthesize() = 0;
        
    protected:
        SourceType mSourceType;
    };

    class SampleSource
    : public Source
    {
    public:
        struct SampleEssence
        : Essence
        {
            juce::AudioSampleBuffer* audioSampleBuffer;
            size_t position;
        };
        
        SampleSource(SampleEssence* essence);
        ~SampleSource() override = default;
        
        size_t getLastPosition() const override;
        
        double synthesize() override;

    private:
        juce::AudioSampleBuffer* mAudioSampleBuffer;
        size_t mPosition {0};
    };

    class SinewaveSource
    : public Source
    {
    public:
        struct OscillatorEssence
        : Essence
        {
            double frequency;
        };
        
        SinewaveSource(OscillatorEssence* essence);
        ~SinewaveSource() override = default;
        
        double synthesize() override;
        
    private:
        double mFrequency {220.0};
        double mCurrentPhase {0.0};
        double mPhasePerSample {0.0};
    };
}
