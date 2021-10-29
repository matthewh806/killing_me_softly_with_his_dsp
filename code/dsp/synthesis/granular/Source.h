#pragma once

#include <iostream>
#include "JuceHeader.h"

class Source
{
public:
    enum class SourceType
    {
        sample,
        synthetic
    };
    
    virtual ~Source() = default;
    
    virtual void init(size_t durationInSamples) = 0;
    virtual double synthesize() = 0;
    
protected:
    SourceType mSourceType;
};

class SampleSource
: public Source
{
public:
    SampleSource(juce::AudioSampleBuffer* sampleBuffer, size_t position);
    ~SampleSource() override = default;
    
    void init(size_t durationInSamples) override;
    double synthesize() override;

private:
    juce::AudioSampleBuffer* mAudioSampleBuffer;
    size_t mDurationInSamples;
    
    size_t mPosition {0};
};
