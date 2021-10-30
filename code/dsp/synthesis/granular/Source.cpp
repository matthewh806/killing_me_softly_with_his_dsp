#include "Source.h"

size_t Source::getLastPosition() const
{
    return 0.0;
}

SampleSource::SampleSource(SampleEssence* essence)
: mAudioSampleBuffer(essence->audioSampleBuffer)
, mPosition(essence->position)
{
    mSourceType = SourceType::sample;
}

size_t SampleSource::getLastPosition() const
{
    return mPosition;
}

double SampleSource::synthesize()
{
    if(mAudioSampleBuffer == nullptr)
    {
        return 0.0;
    }
    
    return mAudioSampleBuffer->getSample(0, static_cast<int>(mPosition++));
}

SinewaveSource::SinewaveSource(OscillatorEssence* essence)
: mFrequency(essence->frequency)
, mPhasePerSample(juce::MathConstants<double>::twoPi / (44100.0 / mFrequency))
{
    mSourceType = SourceType::synthetic;
}

double SinewaveSource::synthesize()
{
    auto const sample = std::sin(mCurrentPhase);
    mCurrentPhase += mPhasePerSample;
    
    return sample;
}
