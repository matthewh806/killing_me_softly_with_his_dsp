#pragma once

#include "SliceManager.h"
#include <JuceHeader.h>
#include <algorithm>

#define RETRIGGER_DIVISION_FACTOR 8

namespace OUS
{
    class BreakbeatAudioSource
    : public PositionableAudioSource
    {
    public:
        BreakbeatAudioSource(juce::AudioFormatManager& formatManager);
        ~BreakbeatAudioSource() override;

        size_t getNumSlices() const;
        int64_t getStartReadPosition() const;
        
        /*
            This returns the playhead position in seconds and takes into
            account reverse play and retrigger modes
         */
        float getPlayheadPosition();
        
        bool isReversing() const;
        bool isRetriggering() const;

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
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        // juce::PositionableAudioSource
        void setNextReadPosition(int64 newPosition) override;
        int64 getNextReadPosition() const override;
        int64 getTotalLength() const override;
        bool isLooping() const override;

        void clear();

    private:
        double mSampleRate{0.0};
        
        SliceManager mSliceManager;

        std::atomic<int64_t> mNextReadPosition{0};
        std::atomic<int64_t> mSliceStartPosition{0};
        std::atomic<int64_t> mNextRetriggerPosition{0};

        std::atomic<float> mSampleChangeThreshold{0.7f};
        std::atomic<float> mReverseSampleThreshold{0.7f};
        std::atomic<float> mRetriggerSampleThreshold{0.7f};

        std::atomic<bool> mRandomPosition{false};
        std::atomic<bool> mRandomDirection{false};

        bool mRetriggering{false};
        bool mReversing{false};

        std::atomic<float> mCrossFade{100.0f}; // ms
    };
} // namespace OUS
