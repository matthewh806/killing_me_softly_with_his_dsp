#include "BreakbeatAudioSource.h"

using namespace OUS;

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

float BreakbeatAudioSource::getPlayheadPosition()
{
    if(mSampleRate <= 0.0)
    {
        return 0.0;
    }
    
    auto playheadPosition = static_cast<size_t>(getNextReadPosition());
    auto slice = mSliceManager.getCurrentSlice();
    auto const sliceStart = std::get<1>(slice);
    auto const sliceEnd = std::get<2>(slice);
    
    if(mReversing)
    {
        playheadPosition = sliceStart + sliceEnd - playheadPosition;
    }
    if(mRetriggering)
    {
        auto const sliceSampleSize = sliceEnd - sliceStart;
        auto const retriggerEndPos = sliceSampleSize / RETRIGGER_DIVISION_FACTOR;
        
        playheadPosition = sliceStart + (playheadPosition % retriggerEndPos);
    }
    
    return static_cast<float>(playheadPosition / mSampleRate);
}

bool BreakbeatAudioSource::isReversing() const
{
    return mReversing;
}

bool BreakbeatAudioSource::isRetriggering() const
{
    return mRetriggering;
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

void BreakbeatAudioSource::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    juce::ignoreUnused(samplesPerBlockExpected);
    mSampleRate = sampleRate;
}

void BreakbeatAudioSource::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
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
    auto sliceStartPosition = std::get<1>(slice);
    auto sliceEndPosition = std::get<2>(slice);
    auto const sliceChangeThreshold = mSampleChangeThreshold.load();
    auto const sliceReverseThreshold = mReverseSampleThreshold.load();
    auto const sliceRetriggerThreshold = mRetriggerSampleThreshold.load();

    // check apply gain ramp
    auto const crossFadeTime = mCrossFade.load();

    auto samplesRemaining = numSamples;
    auto currentPosition = static_cast<size_t>(mNextReadPosition.load());
    auto currentRetriggerPosition = static_cast<size_t>(mNextRetriggerPosition.load());
    auto currentOutputBufferPosition = outputStart;
    while(samplesRemaining > 0)
    {
        sliceStartPosition = std::get<1>(slice);
        sliceEndPosition = std::get<2>(slice);
        
        auto const changePerc = Random::getSystemRandom().nextFloat();
        bool const atSliceEnd = currentPosition >= sliceEndPosition;
        bool const willChange = atSliceEnd && changePerc > sliceChangeThreshold;
        
        if(willChange)
        {
            mRetriggering = false;

            slice = mSliceManager.setRandomSlice();
            sliceStartPosition = std::get<1>(slice);
            sliceEndPosition = std::get<2>(slice);
            
            currentPosition = sliceStartPosition;

            // TODO: This assertion should be enabled
            // Annoyingly this can be true since the loaded slices
            // can be added before the file is loaded and then the
            // last slice which is based on the buffer size sets its end to 0
            //            jassert(sliceEndPosition >= sliceStartPosition);

            auto retriggerPerc = Random::getSystemRandom().nextFloat();
            if(retriggerPerc > sliceRetriggerThreshold)
            {
                currentRetriggerPosition = sliceStartPosition;
                mRetriggering = true;
            }

            auto reversePerc = Random::getSystemRandom().nextFloat();
            if(reversePerc > sliceReverseThreshold)
            {
                mSliceManager.setReverseBufferActive();
                mReversing = true;
            }
            else
            {
                mSliceManager.setForwardBufferActive();
                mReversing = false;
            }
        };

        retainedBuffer = mSliceManager.getActiveBuffer();
        currentPosition = atSliceEnd ? sliceStartPosition : currentPosition;
        auto const readBufferEnd = std::min(sliceEndPosition, currentPosition + static_cast<size_t>(samplesRemaining));
        jassert(currentPosition >= 0 && readBufferEnd >= currentPosition);

        auto numThisTime = 0;
        if(mRetriggering)
        {
            auto const sliceSampleSize = sliceEndPosition - sliceStartPosition;
            auto const retriggerEndPos = sliceStartPosition + sliceSampleSize / RETRIGGER_DIVISION_FACTOR;
            currentRetriggerPosition = (currentRetriggerPosition >= retriggerEndPos) ? sliceStartPosition : currentRetriggerPosition;
            
            numThisTime = std::min(static_cast<int>(retriggerEndPos - currentRetriggerPosition), samplesRemaining);
            numThisTime = std::min(numThisTime, static_cast<int>(readBufferEnd - currentPosition));
            assert(numThisTime >= 0);
            
            for(auto ch = 0; ch < numChannels; ++ch)
            {
                bufferToFill.buffer->copyFrom(ch, outputStart, *retainedBuffer, ch, static_cast<int>(currentRetriggerPosition), static_cast<int>(numThisTime));
            }
        }
        else
        {
            numThisTime = std::min(static_cast<int>(readBufferEnd - currentPosition), samplesRemaining);
            for(auto ch = 0; ch < numChannels; ++ch)
            {
                bufferToFill.buffer->copyFrom(ch, outputStart, *retainedBuffer, ch, static_cast<int>(currentPosition), static_cast<int>(numThisTime));
            }
        }

        // TODO: not quite catching all of the samples to apply the fade to
        auto const crossFadeSamples = static_cast<size_t>((crossFadeTime / 1000.0f) * mSliceManager.getSampleSampleRate());
        if(sliceEndPosition - currentPosition <= crossFadeSamples)
        {
            // crossfade end region!
            auto const samplesToFadeEnd = sliceEndPosition - currentPosition;
            auto const numToApplyGainTo = std::min(static_cast<size_t>(samplesToFadeEnd), static_cast<size_t>(numThisTime));

            // linearly interpolate the value
            auto const xfadeBegin = sliceEndPosition - crossFadeSamples;
            auto const relPos = currentPosition - xfadeBegin;
            auto const curRatioAlong = static_cast<double>(relPos) / crossFadeSamples;

            auto const endRelPos = relPos + numToApplyGainTo;
            auto const endRatioAlong = static_cast<double>(endRelPos) / crossFadeSamples;

            auto const startGain = static_cast<float>(std::min(std::max(0.0, 1.0 - curRatioAlong), 1.0));
            auto const endGain = static_cast<float>(std::min(std::max(0.0, 1.0 - endRatioAlong), 1.0));

            //            std::cout << "Fade out s: " << startGain << ", e: " << endGain << ", L: " << numToApplyGainTo << "\n";

            jassert(startGain >= endGain);
            bufferToFill.buffer->applyGainRamp(static_cast<int>(currentOutputBufferPosition), static_cast<int>(numToApplyGainTo), startGain, endGain);
        }
        else if(currentPosition >= sliceStartPosition && currentPosition < sliceStartPosition + crossFadeSamples)
        {
            // crossfade start!
            auto const samplesToFadeEnd = (sliceStartPosition + crossFadeSamples) - currentPosition;
            auto const numToApplyGainTo = std::min(static_cast<size_t>(samplesToFadeEnd), static_cast<size_t>(numThisTime));

            auto const relPos = static_cast<double>(currentPosition - sliceStartPosition);
            auto const curRatioAlong = relPos / crossFadeSamples;

            auto const endRelPos = relPos + static_cast<double>(numToApplyGainTo);
            auto const endRatioAlong = endRelPos / crossFadeSamples;

            auto const startGain = static_cast<float>(std::min(std::max(0.0, curRatioAlong), 1.0));
            auto const endGain = static_cast<float>(std::min(std::max(0.0, endRatioAlong), 1.0));

            jassert(startGain <= endGain);
            bufferToFill.buffer->applyGainRamp(static_cast<int>(currentOutputBufferPosition), static_cast<int>(numToApplyGainTo), startGain, endGain);

            //            std::cout << "Fade in s: " << startGain << ", e: " << endGain << ", L: " << numToApplyGainTo << "\n";
        }

        samplesRemaining -= numThisTime;
        currentRetriggerPosition += static_cast<size_t>(numThisTime);
        currentPosition += static_cast<size_t>(numThisTime);
        currentOutputBufferPosition += static_cast<size_t>(numThisTime);
    }

    mNextReadPosition.exchange(static_cast<int64_t>(currentPosition));
    mNextRetriggerPosition.exchange(static_cast<int64_t>(currentRetriggerPosition));
    mSliceStartPosition.exchange(static_cast<int64_t>(sliceStartPosition));
}

void BreakbeatAudioSource::releaseResources()
{
}

void BreakbeatAudioSource::setNextReadPosition(int64 newPosition)
{
    mNextReadPosition = newPosition;
    mSliceStartPosition = newPosition;
    mNextRetriggerPosition = newPosition;
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
