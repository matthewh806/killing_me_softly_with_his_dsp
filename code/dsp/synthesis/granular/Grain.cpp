#include "Grain.h"

Grain::Grain()
: mPosition(0)
, mDuration(0)
, mComplete(true)
, mAudioSampleBuffer(nullptr)
, mEnvelope(0)
{
    
}

Grain::Grain(juce::AudioSampleBuffer* sampleBuffer)
: mPosition(0)
, mDuration(0)
, mComplete(true)
, mAudioSampleBuffer(sampleBuffer)
, mEnvelope(0)
{
    //std::cout << "Created grain: id: " << mUuid.toDashedString() << ", pos: " << mPosition << ", duration: " << mDuration << "\n";
}

Grain::~Grain()
{
    //std::cout << "End of grain: id: " << mUuid.toDashedString() << "\n";
}

void Grain::init(size_t position, size_t duration, juce::AudioSampleBuffer* sampleBuffer)
{
    mPosition = position;
    mDuration = duration;
    mAudioSampleBuffer = sampleBuffer;
    
    mSampleCounter = 0;
    mEnvelope.init(duration);
    mComplete = false;
}

bool Grain::isGrainComplete() const
{
    return mComplete;
}

void Grain::synthesise(AudioBuffer<float>* buffer, int numSamples)
{
    if(mAudioSampleBuffer == nullptr)
    {
        // todo: uh oh...
        buffer->clear();
        return;
    }
    
    auto remaining = numSamples;
    auto outputBufPos = 0;
    while(--remaining > 0 && !mComplete)
    {
        auto const val =  mAudioSampleBuffer->getSample(0, static_cast<int>(mPosition)) * static_cast<float>(mEnvelope.synthesize());
        buffer->setSample(0, outputBufPos, val);
        buffer->setSample(1, outputBufPos, val);
        
        mPosition += 1;
        outputBufPos += 1;
        mSampleCounter += 1;
        
        if(mSampleCounter == mDuration)
        {
            mComplete = true;
        }
    }
}

