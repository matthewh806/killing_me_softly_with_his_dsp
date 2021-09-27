#include "BreakbeatAudioSource.h"

BreakbeatAudioSource::BreakbeatAudioSource()
{
    
}

BreakbeatAudioSource::~BreakbeatAudioSource()
{
    
}

int64_t BreakbeatAudioSource::getNumSamples() const
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mCurrentBuffer);
    if(retainedBuffer == nullptr)
    {
        return 0;
    }
    
    auto* currentAudioBuffer = retainedBuffer->getCurrentAudioSampleBuffer();
    if(currentAudioBuffer == nullptr)
    {
        return 0;
    }
    
    return currentAudioBuffer->getNumSamples();
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

juce::AudioSampleBuffer* BreakbeatAudioSource::getCurrentBuffer()
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mCurrentBuffer);
    if(retainedBuffer == nullptr)
    {
        return nullptr;
    }
    
    return retainedBuffer->getForwardAudioSampleBuffer();
}

juce::AudioSampleBuffer* BreakbeatAudioSource::getOriginalAudioSampleBuffer()
{
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mCurrentFileBuffer);
    if(retainedBuffer == nullptr)
    {
        return nullptr;
    }
    
    return retainedBuffer->getForwardAudioSampleBuffer();
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
    
    ReferenceCountedForwardAndReverseBuffer::Ptr retainedBuffer(mCurrentBuffer);
    
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
            retainedBuffer->updateCurrentSampleBuffer(sliceReverseThreshold);
        };
        
        currentPosition = atSliceEnd ? sliceStartPosition : currentPosition;
        auto const readBufferEnd = std::min(sliceEndPosition, currentPosition + static_cast<int64_t>(samplesRemaining));
        jassert(currentPosition >= 0 && readBufferEnd >= currentPosition);
        
        auto const numThisTime = std::min(static_cast<int>(readBufferEnd - currentPosition), samplesRemaining);
        for(auto ch = 0; ch < numChannels; ++ch)
        {
            bufferToFill.buffer->copyFrom(ch, outputStart, *retainedBuffer->getCurrentAudioSampleBuffer(), ch, static_cast<int>(currentPosition), static_cast<int>(numThisTime));
        }
        
        samplesRemaining -= numThisTime;
        currentPosition += numThisTime;
    }
    
    mNextReadPosition.exchange(currentPosition);
    mSliceStartPosition.exchange(sliceStartPosition);
}

void BreakbeatAudioSource::releaseResources()
{
    mCurrentBuffer = nullptr;
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
    return 0.0;
}

bool BreakbeatAudioSource::isLooping() const
{
    return true;
}

void BreakbeatAudioSource::setOriginalReader(juce::AudioFormatReader* reader)
{
    ReferenceCountedForwardAndReverseBuffer::Ptr newBuffer = new ReferenceCountedForwardAndReverseBuffer("", reader);
    jassert(newBuffer != nullptr);
            
    mCurrentFileBuffer = newBuffer;
    mFileBuffers.add(mCurrentBuffer);

    mNextReadPosition = 0;
    mDuration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    
    updateSliceSizes();
}

void BreakbeatAudioSource::setCurrentReader(juce::AudioFormatReader* reader)
{
    ReferenceCountedForwardAndReverseBuffer::Ptr newBuffer = new ReferenceCountedForwardAndReverseBuffer("", reader);
    jassert(newBuffer != nullptr);
            
    mCurrentBuffer = newBuffer;
    mBuffers.add(mCurrentBuffer);

    mNextReadPosition = 0;
    mDuration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    
    updateSliceSizes();
}

void BreakbeatAudioSource::clearFreeBuffers()
{
    for(auto i = mBuffers.size(); --i >= 0;)
    {
        ReferenceCountedForwardAndReverseBuffer::Ptr buffer(mBuffers.getUnchecked(i));
        
        if(buffer->getReferenceCount() == 2)
        {
            mBuffers.remove(i);
        }
    }
    
    for(auto i = mFileBuffers.size(); --i >= 0;)
    {
        ReferenceCountedForwardAndReverseBuffer::Ptr buffer(mFileBuffers.getUnchecked(i));
        
        if(!buffer)
        {
            continue;
        }
        
        if(buffer->getReferenceCount() == 2)
        {
            mFileBuffers.remove(i);
        }
    }
}

void BreakbeatAudioSource::clear()
{
    mCurrentBuffer = nullptr;
    mCurrentFileBuffer = nullptr;
}

void BreakbeatAudioSource::updateSliceSizes()
{
    mNumSlices = mBlockDivisionFactor.load();
    mSliceSampleSize = roundToInt(getNumSamples() / mBlockDivisionFactor);
    
    jassert(mNumSlices > 0);
    
    setNextReadPosition(0);
}
