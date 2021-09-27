#pragma once

#include <JuceHeader.h>
#include "SampleManager.h"

class BreakbeatAudioSource
: public PositionableAudioSource
{
public:
    BreakbeatAudioSource(SampleManager& sampleManager);
    ~BreakbeatAudioSource() override;
    
    int getNumSlices() const;
    int64_t getSliceSize() const;
    int64_t getStartReadPosition() const;
    
    void setSampleChangeThreshold(float threshold);
    void setReverseSampleThreshold(float threshold);
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
    
    void updateSliceSizes();
    
private:
    
    SampleManager& mSampleManager;
    
    std::atomic<int64_t> mNextReadPosition {0};
    std::atomic<int64_t> mSliceStartPosition {0};
    
    std::atomic<float> mSampleChangeThreshold {0.7f};
    std::atomic<float> mReverseSampleThreshold {0.7f};
    
    std::atomic<bool> mRandomPosition {false};
    std::atomic<bool> mRandomDirection {false};
    
    std::atomic<int> mNumSlices {1};
    std::atomic<int> mSliceSampleSize {1}; // in samples
    std::atomic<int> mBlockDivisionFactor {1};
    
    std::atomic<float> mCrossFade {100.0f};  //ms
};
