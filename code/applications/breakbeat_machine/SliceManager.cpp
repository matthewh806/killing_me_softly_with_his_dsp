#include "SliceManager.h"
#include <algorithm>

SliceManager::SliceManager(juce::AudioFormatManager& formatManager, Method sliceMethod)
: SampleManager(formatManager)
, mSliceMethod(sliceMethod)
{
    performSlice();
}

SliceManager::Method SliceManager::getSliceMethod()
{
    return mSliceMethod;
}

void SliceManager::setSliceMethod(Method method)
{
    if(method != mSliceMethod)
    {
        mSliceMethod = method;
        performSlice();
    }
}

void SliceManager::setThreshold(float threshold)
{
    mThreshold = threshold;
    
    if(mSliceMethod == Method::transients)
    {
        performSlice();
    }
}

void SliceManager::setDivisions(float divisions)
{
    mDivisions = divisions;
    
    if(mSliceMethod == Method::divisions)
    {
        performSlice();
    }
}

void SliceManager::addSlice(size_t position)
{
    
    if(std::find(mSlicePositions.begin(), mSlicePositions.end(), position) == mSlicePositions.end())
    {
        mSlicePositions.push_back(position);
        if(mSliceMethod == Method::manual)
        {
            performSlice();
        }
    }
}

void SliceManager::deleteSlice(size_t position)
{
    juce::ignoreUnused(position);
}

// TODO: move slice?

size_t SliceManager::getCurrentSliceIndex() const
{
    return mCurrentSliceIndex;
}

SliceManager::Slice SliceManager::getCurrentSlice() const
{
    return mCurrentSlice;
}

// Sets the current slice to a random one and returns it
SliceManager::Slice SliceManager::setRandomSlice()
{
    // this is a hack to prevent a threading issue
    if(mSlicePositions.size() == 0)
    {
        return mCurrentSlice;
    }
 
    // TODO: better random approach
    auto const sliceIndex = static_cast<size_t>(Random::getSystemRandom().nextInt(static_cast<int>(mSlicePositions.size())));
    auto const sliceStart = mSlicePositions[sliceIndex];
    auto const isFinalSlice = sliceIndex == mSlicePositions.size() - 1;
    auto const sliceEnd = isFinalSlice ? getBufferNumSamples() - 1  : mSlicePositions[sliceIndex + 1];
    
    jassert(sliceStart < sliceEnd);
    
    mCurrentSlice = {sliceStart, sliceEnd};
    mCurrentSliceIndex = sliceIndex;
    
    sendChangeMessage();
    
    return mCurrentSlice;
}

std::vector<size_t> const& SliceManager::getSlices() const
{
    return mSlicePositions;
}

size_t SliceManager::getNumberOfSlices() const
{
    return mSlicePositions.size();
}

void SliceManager::performSlice()
{
    auto const bufferLength = getBufferNumSamples();
    if(bufferLength <= 0)
    {
        // no file loaded
        return;
    }
    
    if(mSliceMethod == divisions)
    {
        mSlicePositions.clear();
        
        // divide up
        auto const sliceSampleSize = static_cast<size_t>(static_cast<double>(bufferLength) / mDivisions);
        auto const numSlices = bufferLength / sliceSampleSize;
        
        jassert(numSlices > 0);
        
        mSlicePositions.resize(numSlices);
        for(size_t i = 0; i < numSlices; ++i)
        {
            mSlicePositions[i] = i * sliceSampleSize;
        }
        
        auto const sliceStart = mSlicePositions[0];
        auto const sliceEnd = mSlicePositions.size() == 1 ? getBufferNumSamples() : mSlicePositions[1];
        jassert(sliceStart < sliceEnd);
        
        mCurrentSliceIndex = 0;
        mCurrentSlice = {sliceStart, sliceEnd};
    }
    else if(mSliceMethod == transients)
    {
        /*
         TODO:
         This happens on the message thread and can cause problems
         for the audio thread still trying to access it 
         */
        mSlicePositions.clear();
        
        AudioAnalyser::DetectionSettings detectionSettings;
        detectionSettings.sampleRate = static_cast<int>(getSampleSampleRate());
        detectionSettings.threshold = mThreshold;
        
        mSlicePositions = AudioAnalyser::getOnsetPositions(*getActiveBuffer(), detectionSettings);
        auto const numSlices = mSlicePositions.size();
        jassert(numSlices > 0);
        
        auto const sliceStart = mSlicePositions[0];
        auto const sliceEnd = mSlicePositions.size() == 1 ? getBufferNumSamples() : mSlicePositions[1];
        jassert(sliceStart < sliceEnd);
        
        mCurrentSliceIndex = 0;
        mCurrentSlice = {sliceStart, sliceEnd};
    }
    else if(mSliceMethod == manual)
    {
        // hmmm? check slice still exists - update index etc as necessary
        
        // sort in ascending order as we add them into a random position
        std::sort(mSlicePositions.begin(), mSlicePositions.end());
    }
    
    sendChangeMessage();
}
