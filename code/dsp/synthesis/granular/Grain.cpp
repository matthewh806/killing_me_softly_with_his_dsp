#include "Grain.h"

Grain::Grain()
: mDuration(0)
, mComplete(true)
, mSource(nullptr)
, mEnvelope(nullptr)
{
    
}

Grain::~Grain()
{
    //std::cout << "End of grain: id: " << mUuid.toDashedString() << "\n";
}

void Grain::init(size_t position, size_t duration, double frequency, juce::AudioSampleBuffer* sampleBuffer, Source::SourceType sourceType, Envelope::EnvelopeType envelopeType)
{
    mDuration = duration;
    
    mSampleCounter = 0;
    
    switch(sourceType)
    {
        case Source::SourceType::sample:
        {
            mSource = std::make_unique<SampleSource>(sampleBuffer, position);
        }
            break;
        case Source::SourceType::synthetic:
        {
            mSource = std::make_unique<SinewaveSource>(frequency);
        }
            break;
    }
    
    if(mSource == nullptr)
    {
        std::cerr << "Failed to create sample source!\n";
        return;
    }
    mSource->init(duration);
    
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
    if(mSource == nullptr || mEnvelope == nullptr)
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
        auto const val =  static_cast<float>(mSource->synthesize()) * static_cast<float>(mEnvelope->synthesize());
        buffer->setSample(0, outputBufPos, val);
        buffer->setSample(1, outputBufPos, val);
        
        outputBufPos += 1;
        mSampleCounter += 1;
        
        if(mSampleCounter == mDuration)
        {
            mComplete = true;
        }
    }
}

