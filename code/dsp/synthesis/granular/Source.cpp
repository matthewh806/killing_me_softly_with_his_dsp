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
