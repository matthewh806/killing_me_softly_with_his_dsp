#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include "SliceManager.h"

#define RETRIGGER_DIVISION_FACTOR 8

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
    void setSliceMethod(SliceManager::Method method);
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
    std::atomic<int64_t> mNextRetriggerPosition {0};
    
    std::atomic<float> mSampleChangeThreshold {0.7f};
    std::atomic<float> mReverseSampleThreshold {0.7f};
    std::atomic<float> mRetriggerSampleThreshold {0.7f};
    
    std::atomic<bool> mRandomPosition {false};
    std::atomic<bool> mRandomDirection {false};
    
    bool mRetriggering {false};
    
    std::atomic<float> mCrossFade {100.0f};  //ms
};
