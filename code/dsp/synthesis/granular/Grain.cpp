#include "Grain.h"

Grain::Grain()
: mPosition(0)
, mDuration(0)
, mComplete(true)
, mAudioSampleBuffer(nullptr)
, mEnvelope(nullptr)
{
    
}

Grain::~Grain()
{
    //std::cout << "End of grain: id: " << mUuid.toDashedString() << "\n";
}

void Grain::init(size_t position, size_t duration, juce::AudioSampleBuffer* sampleBuffer, Envelope::EnvelopeType envelopeType)
{
    mPosition = position;
    mDuration = duration;
    mAudioSampleBuffer = sampleBuffer;
    
    mSampleCounter = 0;
    
    switch(envelopeType)
    {
        case Envelope::EnvelopeType::trapezoidal:
        {
            mEnvelope = std::make_unique<TrapezoidalEnvelope>(duration);
            break;
        }
        case Envelope::EnvelopeType::parabolic:
        {
            mEnvelope = std::make_unique<ParabolicEnvelope>(duration);
        }
            
        default:
            // todo: maybe just unity gain?
            mEnvelope = std::make_unique<TrapezoidalEnvelope>(duration);
            break;
    }
    if(mEnvelope == nullptr)
    {
        std::cerr << "Failed to create envelope!\n";
        return;
    }
    
    mEnvelope->init(duration);
    mComplete = false;
}

bool Grain::isGrainComplete() const
{
    return mComplete;
}

void Grain::synthesise(AudioBuffer<float>* buffer, int numSamples)
{
    if(mAudioSampleBuffer == nullptr || mEnvelope == nullptr)
    {
        // todo: uh oh...
        std::cerr << "Error synthesising grain!\n";
        buffer->clear();
        return;
    }
    
    if(mComplete)
    {
        return;
    }
    
    auto remaining = numSamples;
    auto outputBufPos = 0;
    while(--remaining > 0 && !mComplete)
    {
        auto const val =  mAudioSampleBuffer->getSample(0, static_cast<int>(mPosition)) * static_cast<float>(mEnvelope->synthesize());
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

