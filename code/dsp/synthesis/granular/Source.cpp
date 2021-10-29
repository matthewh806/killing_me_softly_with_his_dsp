#include "Source.h"

SampleSource::SampleSource(juce::AudioSampleBuffer* sampleBuffer, size_t position)
: mAudioSampleBuffer(sampleBuffer)
, mPosition(position)
{
    mSourceType = SourceType::sample;
}

void SampleSource::init(size_t durationInSamples)
{
    mDurationInSamples = durationInSamples;
}

double SampleSource::synthesize()
{
    if(mAudioSampleBuffer == nullptr)
    {
        return 0.0;
    }
    
    return mAudioSampleBuffer->getSample(0, static_cast<int>(mPosition++));
}

SinewaveSource::SinewaveSource(double frequency)
: mFrequency(frequency)
, mPhasePerSample(juce::MathConstants<double>::twoPi / (44100.0 / mFrequency))
{
    mSourceType = SourceType::synthetic;
}

void SinewaveSource::init(size_t durationInSamples)
{
    // todo: this method is pointless! DELETE DELETE DELETE!!
}

double SinewaveSource::synthesize()
{
    auto const sample = std::sin(mCurrentPhase);
    mCurrentPhase += mPhasePerSample;
    
    return sample;
}
