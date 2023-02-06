#pragma once

#include "JuceHeader.h"
#include "../../dependencies/rubberband/rubberband/RubberBandStretcher.h"
#include "../../core/RingBuffer.h"

namespace OUS
{
    class OfflineStretchProcessor : public juce::ThreadWithProgressWindow
    {
    public:
        OfflineStretchProcessor(TemporaryFile& file, juce::AudioSampleBuffer& stretchSrc, float stretchFactor, float pitchFactor, double sampleRate, std::function<void()> onThreadComplete = nullptr);
        void run() override;
        
        void threadComplete (bool userPressedCancel) override;
        
    private:
        juce::TemporaryFile& mFile;
        
        juce::AudioSampleBuffer mStretchSrc;
        float mStretchFactor = 1.0f;
        float mPitchShiftFactor = 1.0f;
        
        std::unique_ptr<RubberBand::RubberBandStretcher> mRubberBandStretcher;
        juce::AudioSampleBuffer mStretchedBuffer;
        
        std::function<void()> mOnThreadComplete;
    };
}

