#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include "SampleManager.h"
#include "../../analysis/AudioAnalyser.h"

class SliceManager
: public SampleManager
, public juce::ChangeBroadcaster
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
    
    SliceManager(juce::AudioFormatManager& formatManager, Method sliceMethod = Method::divisions)
    : SampleManager(formatManager)
    , mSliceMethod(sliceMethod)
    {
        performSlice();
    }
    
    ~SliceManager() = default;
    
    Method getSliceMethod()
    {
        return mSliceMethod;
    }
    
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
    
    size_t getCurrentSliceIndex() const
    {
        return mCurrentSliceIndex;
    }
    
    Slice getCurrentSlice() const
    {
        return mCurrentSlice;
    }
    
    // Sets the current slice to a random one and returns it
    Slice setRandomSlice()
    {
        // TODO: better random approach
        auto const sliceIndex = static_cast<size_t>(Random::getSystemRandom().nextInt(static_cast<int>(mSlicePositions.size())));
        auto const sliceStart = mSlicePositions[sliceIndex];
        auto const isFinalSlice = sliceIndex == mSlicePositions.size() - 1;
        auto const sliceEnd = isFinalSlice ? getBufferNumSamples() : mSlicePositions[sliceIndex + 1];
        
        jassert(sliceStart < sliceEnd);
        
        mCurrentSlice = {sliceStart, sliceEnd};
        mCurrentSliceIndex = sliceIndex;
        
        sendChangeMessage();
        
        return mCurrentSlice;
    }
    
    std::vector<size_t> const& getSlices() const
    {
        return mSlicePositions;
    }
    
    size_t getNumberOfSlices() const
    {
        return mSlicePositions.size();
    }
    
    void performSlice()
    {
        auto const bufferLength = getBufferNumSamples();
        if(bufferLength <= 0)
        {
            // no file loaded
            return;
        }
        
        mSlicePositions.clear();
        if(mSliceMethod == divisions)
        {
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
        else
        {
            AudioAnalyser::DetectionSettings detectionSettings;
            detectionSettings.sampleRate = static_cast<int>(getSampleSampleRate());
            detectionSettings.threshold = mThreshold;
            
            mSlicePositions = AudioAnalyser::getOnsetPositions(*getActiveBuffer(), detectionSettings);
            auto const numSlices = mSlicePositions.size();
            jassert(numSlices > 0);
            
            std::cout << "Onset points found: " << numSlices << "\n";
            for(auto const& onset : mSlicePositions)
            {
                std::cout << " o : " << onset << "\n";
            }
            
            auto const sliceStart = mSlicePositions[0];
            auto const sliceEnd = mSlicePositions.size() == 1 ? getBufferNumSamples() : mSlicePositions[1];
            jassert(sliceStart < sliceEnd);
            
            mCurrentSliceIndex = 0;
            mCurrentSlice = {sliceStart, sliceEnd};

        }
        
        sendChangeMessage();
    }
    
private:
    
    size_t mCurrentSliceIndex {0};
    Slice mCurrentSlice;
    
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
    BreakbeatAudioSource(juce::AudioFormatManager& formatManager);
    ~BreakbeatAudioSource() override;
    
    size_t getNumSlices() const;
    int64_t getStartReadPosition() const;
    
    SliceManager& getSliceManager();
    
    void setSampleChangeThreshold(float threshold);
    void setReverseSampleThreshold(float threshold);
    void setRetriggerSampleThreshold(float threshold);
    void setBlockDivisionFactor(int factor);
    void setTransientDetectionThreshold(float threshold);
    
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
    
    SliceManager mSliceManager;
    
    std::atomic<int64_t> mNextReadPosition {0};
    std::atomic<int64_t> mSliceStartPosition {0};
    
    std::atomic<float> mSampleChangeThreshold {0.7f};
    std::atomic<float> mReverseSampleThreshold {0.7f};
    std::atomic<float> mRetriggerSampleThreshold {0.7f};
    
    std::atomic<bool> mRandomPosition {false};
    std::atomic<bool> mRandomDirection {false};
    
    std::atomic<float> mCrossFade {100.0f};  //ms
};
