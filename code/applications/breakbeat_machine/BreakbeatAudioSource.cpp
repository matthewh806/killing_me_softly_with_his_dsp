#include "BreakbeatAudioSource.h"

BreakbeatAudioSource::BreakbeatAudioSource(SampleManager& sampleManager)
: mSampleManager(sampleManager)
{
    
}

BreakbeatAudioSource::~BreakbeatAudioSource()
{
    
}

int BreakbeatAudioSource::getNumSlices() const
{
    return mNumSlices.load();
}

int64_t BreakbeatAudioSource::getSliceSize() const
{
    return mSliceSampleSize.load();
}

int64_t BreakbeatAudioSource::getStartReadPosition() const
{
    return mSliceStartPosition.load();
}

void BreakbeatAudioSource::setSampleChangeThreshold(float threshold)
{
    mSampleChangeThreshold.exchange(threshold);
}

void BreakbeatAudioSource::setReverseSampleThreshold(float threshold)
{
    mReverseSampleThreshold.exchange(threshold);
}

void BreakbeatAudioSource::setBlockDivisionFactor(int factor)
{
    mBlockDivisionFactor.exchange(factor);
    updateSliceSizes();
}

void BreakbeatAudioSource::toggleRandomDirection()
{
    auto const status = mRandomDirection.load();
    mRandomDirection.exchange(!status);
}

void BreakbeatAudioSource::prepareToPlay (int, double)
{
    
}

void BreakbeatAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    
    juce::AudioSampleBuffer* retainedBuffer = mSampleManager.getActiveBuffer();
    if(retainedBuffer == nullptr)
    {
        return;
    }
    
    auto const numChannels = bufferToFill.buffer->getNumChannels();
    auto const numSamples = bufferToFill.numSamples;
    auto const outputStart = bufferToFill.startSample;
    
    auto sliceStartPosition = mSliceStartPosition.load();
    auto const sliceSampleSize = mSliceSampleSize.load();
    auto sliceEndPosition = sliceStartPosition + sliceSampleSize;
    auto const numSlices = mNumSlices.load();
    auto const sliceChangeThreshold = mSampleChangeThreshold.load();
    auto const sliceReverseThreshold = mReverseSampleThreshold.load();
    
    jassert(sliceSampleSize > 0);

    auto samplesRemaining = numSamples;
    auto currentPosition = mNextReadPosition.load();
    while(samplesRemaining > 0)
    {
        auto const changePerc = Random::getSystemRandom().nextFloat();
        
        jassert(currentPosition <= sliceEndPosition);
        bool const atSliceEnd = currentPosition == sliceEndPosition;
        bool const willChange = atSliceEnd && changePerc > sliceChangeThreshold;
        
        if(willChange)
        {
            sliceStartPosition = numSlices > 1 ? Random::getSystemRandom().nextInt(numSlices) * sliceSampleSize : sliceStartPosition;
            sliceEndPosition = sliceStartPosition + sliceSampleSize;
            jassert(sliceEndPosition >= sliceStartPosition);
//            retainedBuffer->updateCurrentSampleBuffer(sliceReverseThreshold);
        };
        
        currentPosition = atSliceEnd ? sliceStartPosition : currentPosition;
        auto const readBufferEnd = std::min(sliceEndPosition, currentPosition + static_cast<int64_t>(samplesRemaining));
        jassert(currentPosition >= 0 && readBufferEnd >= currentPosition);
        
        auto const numThisTime = std::min(static_cast<int>(readBufferEnd - currentPosition), samplesRemaining);
        for(auto ch = 0; ch < numChannels; ++ch)
        {
            bufferToFill.buffer->copyFrom(ch, outputStart, *retainedBuffer, ch, static_cast<int>(currentPosition), static_cast<int>(numThisTime));
        }
        
        samplesRemaining -= numThisTime;
        currentPosition += numThisTime;
    }
    
    mNextReadPosition.exchange(currentPosition);
    mSliceStartPosition.exchange(sliceStartPosition);
}

void BreakbeatAudioSource::releaseResources()
{
    
}

void BreakbeatAudioSource::setNextReadPosition (int64 newPosition)
{
    mNextReadPosition = newPosition;
    mSliceStartPosition = newPosition;
}

int64 BreakbeatAudioSource::getNextReadPosition() const
{
    return mNextReadPosition.load();
}

int64 BreakbeatAudioSource::getTotalLength() const
{
    return static_cast<int64>(mSampleManager.getBufferNumSamples());
}

bool BreakbeatAudioSource::isLooping() const
{
    return true;
}

void BreakbeatAudioSource::clear()
{
}

void BreakbeatAudioSource::updateSliceSizes()
{
    mNumSlices = mBlockDivisionFactor.load();
    mSliceSampleSize = static_cast<int>(static_cast<double>(mSampleManager.getBufferNumSamples()) / mBlockDivisionFactor);
    
    jassert(mNumSlices > 0);
    setNextReadPosition(0);
}
