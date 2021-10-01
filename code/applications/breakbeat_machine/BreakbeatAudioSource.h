#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include "SampleManager.h"

class SliceManager
{
public:
        
    // this tuple should be <start, end>
    // where end is defined either by the pos of the next marker or the end of the file
    using Slice = std::tuple<size_t, size_t>;
    
    enum Method
    {
        divisions,
        transients
    };
    
    SliceManager(SampleManager& sampleManager, Method sliceMethod = Method::divisions)
    : mSampleManager(sampleManager)
    , mSliceMethod(sliceMethod)
    {
        performSlice();
    }
    
    ~SliceManager() = default;
    
    void setSliceMethod(Method method)
    {
        if(method != mSliceMethod)
        {
            mSliceMethod = method;
            performSlice();
        }
    }
    
    void setThreshold(float threshold)
    {
        mThreshold = threshold;
        
        if(mSliceMethod == Method::transients)
        {
            performSlice();
        }
    }
    
    void setDivisions(float divisions)
    {
        mDivisions = divisions;
        
        if(mSliceMethod == Method::divisions)
        {
            performSlice();
        }
    }
    
    Slice getCurrentSlice() const
    {
        return mCurrentSlice;
    }
    
    // Sets the current slice to a random one and returns it
    Slice setRandomSlice()
    {
        auto const sliceIndex = static_cast<size_t>(Random::getSystemRandom().nextInt(mSlicePositions.size()));
        auto const sliceStart = mSlicePositions[sliceIndex];
        auto const isFinalSlice = sliceIndex == mSlicePositions.size() - 1;
        auto const sliceEnd = isFinalSlice ? mSampleManager.getBufferNumSamples() : mSlicePositions[sliceIndex + 1];
        
        jassert(sliceStart < sliceEnd);
        
        mCurrentSlice = {sliceStart, sliceEnd};
        
        return mCurrentSlice;
    }
    
    std::vector<size_t>& getSlices()
    {
        return mSlicePositions;
    }
    
    size_t getNumberOfSlices() const
    {
        return mSlicePositions.size();
    }
    
    void performSlice()
    {
        auto const bufferLength = mSampleManager.getBufferNumSamples();
        if(bufferLength <= 0)
        {
            // no file loaded
            return;
        }
        
        mSlicePositions.clear();
        if(mSliceMethod == divisions)
        {
            // divide up
            auto const sliceSampleSize = static_cast<int>(static_cast<double>(bufferLength) / mDivisions);
            auto const numSlices = bufferLength / sliceSampleSize;
            
            jassert(numSlices > 0);
            
            mSlicePositions.resize(numSlices);
            for(auto i = 0; i < numSlices; ++i)
            {
                mSlicePositions[i] = i * sliceSampleSize;
            }
            
            auto const sliceStart = mSlicePositions[0];
            auto const sliceEnd = mSlicePositions.size() == 1 ? mSampleManager.getBufferNumSamples() : mSlicePositions[1];
            jassert(sliceStart < sliceEnd);
            
            mCurrentSlice = {sliceStart, sliceEnd};
        }
        else
        {
            // do nothing for now :(
        }
    }
    
private:
    
    Slice mCurrentSlice;
    
    SampleManager& mSampleManager;
    Method mSliceMethod;
    
    float mThreshold;
    float mDivisions {1};
    
    // how to make this thread safe? std vector not trivally copyable -> so cant be atomic
    std::vector<size_t> mSlicePositions;
};

class BreakbeatAudioSource
: public PositionableAudioSource
{
public:
    BreakbeatAudioSource(SampleManager& sampleManager);
    ~BreakbeatAudioSource() override;
    
    size_t getNumSlices() const;
    int64_t getStartReadPosition() const;
    
    SliceManager& getSliceManager();
    
    void setSampleChangeThreshold(float threshold);
    void setReverseSampleThreshold(float threshold);
    void setRetriggerSampleThreshold(float threshold);
    void setBlockDivisionFactor(int factor);
    
    void setCrossFade(float xfade);
    
    void toggleRandomPosition();
    void toggleRandomDirection();
    
    // juce::AudioAppComponent
    void prepareToPlay (int, double) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    // juce::PositionableAudioSource
    void setNextReadPosition (int64 newPosition) override;
    int64 getNextReadPosition() const override;
    int64 getTotalLength() const override;
    bool isLooping() const override;
    
    void clear();
    
private:
    
    SampleManager& mSampleManager;
    SliceManager mSliceManager {mSampleManager};
    
    std::atomic<int64_t> mNextReadPosition {0};
    
    std::atomic<int64_t> mSliceStartPosition {0};
    
    std::atomic<float> mSampleChangeThreshold {0.7f};
    std::atomic<float> mReverseSampleThreshold {0.7f};
    std::atomic<float> mRetriggerSampleThreshold {0.7f};
    
    std::atomic<bool> mRandomPosition {false};
    std::atomic<bool> mRandomDirection {false};
    
    std::atomic<int> mNumSlices {1};
    std::atomic<int> mSliceSampleSize {1}; // in samples
    std::atomic<int> mBlockDivisionFactor {1};
    
    std::atomic<float> mCrossFade {100.0f};  //ms
};
