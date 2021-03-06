#include "BreakbeatAudioSource.h"

BreakbeatAudioSource::BreakbeatAudioSource(juce::AudioFormatManager& formatManager)
: mSliceManager(formatManager)
{
    
}

BreakbeatAudioSource::~BreakbeatAudioSource()
{
    
}

size_t BreakbeatAudioSource::getNumSlices() const
{
    return mSliceManager.getNumberOfSlices();
}

SliceManager& BreakbeatAudioSource::getSliceManager()
{
    return mSliceManager;
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

void BreakbeatAudioSource::setRetriggerSampleThreshold(float threshold)
{
    mRetriggerSampleThreshold.exchange(threshold);
}

void BreakbeatAudioSource::setBlockDivisionFactor(int factor)
{
    mSliceManager.setDivisions(factor);
    setNextReadPosition(0);
}

void BreakbeatAudioSource::setSliceMethod(SliceManager::Method method)
{
    mSliceManager.setSliceMethod(method);
    setNextReadPosition(0);
}

void BreakbeatAudioSource::setTransientDetectionThreshold(float threshold)
{
    mSliceManager.setThreshold(threshold);
    setNextReadPosition(0);
}

void BreakbeatAudioSource::setCrossFade(float xfade)
{
    mCrossFade.exchange(xfade);
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
    
    juce::AudioSampleBuffer* retainedBuffer = mSliceManager.getActiveBuffer();
    if(retainedBuffer == nullptr)
    {
        return;
    }
    
    auto const numChannels = bufferToFill.buffer->getNumChannels();
    auto const numSamples = bufferToFill.numSamples;
    auto const outputStart = bufferToFill.startSample;
    
    auto slice = mSliceManager.getCurrentSlice();
    auto sliceStartPosition = std::get<0>(slice);
    auto sliceEndPosition = std::get<1>(slice);
    auto const sliceChangeThreshold = mSampleChangeThreshold.load();
    auto const sliceReverseThreshold = mReverseSampleThreshold.load();
//    auto const sliceRetriggerThreshold = mRetriggerSampleThreshold.load();
    
    // check apply gain ramp
    auto const crossFadeTime = mCrossFade.load();
    auto const crossFadeSamples = static_cast<size_t>((crossFadeTime / 1000.0f) * mSliceManager.getSampleSampleRate());

    auto samplesRemaining = numSamples;
    auto currentPosition = static_cast<size_t>(mNextReadPosition.load());
    while(samplesRemaining > 0)
    {
        auto const changePerc = Random::getSystemRandom().nextFloat();
        
        jassert(currentPosition <= sliceEndPosition);
        bool const atSliceEnd = currentPosition == sliceEndPosition;
        bool const willChange = atSliceEnd && changePerc > sliceChangeThreshold;
        
        if(willChange)
        {
            slice = mSliceManager.setRandomSlice();
            sliceStartPosition = std::get<0>(slice);
            sliceEndPosition = std::get<1>(slice);
            
//            auto retriggerPerc = Random::getSystemRandom().nextFloat();
//            auto sliceSize = sliceSampleSize;
//            if(retriggerPerc > 1.0 - sliceRetriggerThreshold)
//            {
//                sliceSize /= 4;
//            }
//
//            sliceEndPosition = sliceStartPosition + sliceSize;
            jassert(sliceEndPosition >= sliceStartPosition);
        
            auto reversePerc = Random::getSystemRandom().nextFloat();
            if(reversePerc > sliceReverseThreshold)
            {
                mSliceManager.setReverseBufferActive();
            }
            else
            {
                mSliceManager.setForwardBufferActive();
            }
        };
        
        currentPosition = atSliceEnd ? sliceStartPosition : currentPosition;
        auto const readBufferEnd = std::min(sliceEndPosition, currentPosition + static_cast<size_t>(samplesRemaining));
        jassert(currentPosition >= 0 && readBufferEnd >= currentPosition);
        
        auto const numThisTime = static_cast<size_t>(std::min(static_cast<int>(readBufferEnd - currentPosition), samplesRemaining));
        for(auto ch = 0; ch < numChannels; ++ch)
        {
            bufferToFill.buffer->copyFrom(ch, outputStart, *retainedBuffer, ch, static_cast<int>(currentPosition), static_cast<int>(numThisTime));
        }
        
        // TODO: not quite catching all of the samples to apply the fade to
        if(sliceEndPosition - currentPosition < crossFadeSamples)
        {
            // crossfade end region!
            auto const samplesToEnd = sliceEndPosition - currentPosition;
            auto const numToApplyGainTo = std::min(static_cast<size_t>(samplesToEnd), numThisTime);
            
            // linearly interpolate the value
            auto const xfadeBegin = sliceEndPosition - crossFadeSamples;
            auto const relPos = currentPosition - xfadeBegin;
            auto const curRatioAlong = static_cast<double>(relPos) / crossFadeSamples;
            
            auto const endRelPos = (currentPosition + numToApplyGainTo) - xfadeBegin;
            auto const endRatioAlong = static_cast<double>(endRelPos) / crossFadeSamples;
            
            auto const startGain = static_cast<float>(std::min(std::max(0.0, 1.0 - curRatioAlong), 1.0));
            auto const endGain = static_cast<float>(std::min(std::max(0.0, 1.0 - endRatioAlong), 1.0));
            
//            std::cout << "Fade out s: " << startGain << "e: " << endGain << "\n";
            
            jassert(startGain >= endGain);
            bufferToFill.buffer->applyGainRamp(0, static_cast<int>(numToApplyGainTo), startGain, endGain);
        }
        else if(currentPosition < crossFadeSamples)
        {
            // crossfade start!
            auto const samplesToEnd = crossFadeSamples - currentPosition;
            auto const numToApplyGainTo = std::min(static_cast<size_t>(samplesToEnd), numThisTime);
            
            auto const relPos = static_cast<double>(currentPosition);
            auto const curRatioAlong = relPos / crossFadeSamples;
            
            auto const endRelPos = static_cast<double>(currentPosition + numToApplyGainTo);
            auto const endRatioAlong = endRelPos / crossFadeSamples;
            
            auto const startGain = static_cast<float>(std::min(std::max(0.0, curRatioAlong), 1.0));
            auto const endGain = static_cast<float>(std::min(std::max(0.0, endRatioAlong), 1.0));
            
            jassert(startGain <= endGain);
            bufferToFill.buffer->applyGainRamp(0, static_cast<int>(numToApplyGainTo), startGain, endGain);
            
//            std::cout << "Fade in s: " << startGain << "e: " << endGain << "\n";
        }
        
        samplesRemaining -= numThisTime;
        currentPosition += numThisTime;
    }
    
    mNextReadPosition.exchange(static_cast<int64_t>(currentPosition));
    mSliceStartPosition.exchange(static_cast<int64_t>(sliceStartPosition));
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
    return static_cast<int64>(mSliceManager.getBufferNumSamples());
}

bool BreakbeatAudioSource::isLooping() const
{
    return true;
}

void BreakbeatAudioSource::clear()
{
}
