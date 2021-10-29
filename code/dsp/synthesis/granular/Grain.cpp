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

void Grain::init(size_t duration, Source::Essence* sourceEssence, Envelope::Essence* envelopeEssence)
{
    mDuration = duration;
    mSampleCounter = 0;
    
    if(dynamic_cast<SampleSource::SampleEssence*>(sourceEssence) != nullptr)
    {
        auto essence = dynamic_cast<SampleSource::SampleEssence*>(sourceEssence);
        mSource = std::make_unique<SampleSource>(essence);
    }
    else if(dynamic_cast<SinewaveSource::OscillatorEssence*>(sourceEssence))
    {
        auto essence = dynamic_cast<SinewaveSource::OscillatorEssence*>(sourceEssence);
        mSource = std::make_unique<SinewaveSource>(essence);
    }
    
    if(mSource == nullptr)
    {
        std::cerr << "Failed to create sample source!\n";
        return;
    }
    mSource->init(duration);
    
    if(dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envelopeEssence) != nullptr)
    {
        auto essence = dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envelopeEssence);
        mEnvelope = std::make_unique<TrapezoidalEnvelope>(mDuration, essence);
    }
    else if(dynamic_cast<ParabolicEnvelope::ParabolicEssence*>(envelopeEssence) != nullptr)
    {
        auto essence = dynamic_cast<ParabolicEnvelope::ParabolicEssence*>(envelopeEssence);
        mEnvelope = std::make_unique<ParabolicEnvelope>(mDuration, essence);
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

