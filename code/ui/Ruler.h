#pragma once

#include <JuceHeader.h>

namespace OUS
{
    // Horizontal only for now
    // Sample display only for now
    class Ruler
    : public juce::Component
    {
    public:
        Ruler();
        ~Ruler() override = default;
        
        void setSampleRate(double sampleRate);
        void setTotalRange(juce::Range<float> totalRange);
        void setVisibleRange(juce::Range<float> visibleRange);
        
        void paint(juce::Graphics& g) override;
    private:
        double mSampleRate;
        
        juce::Range<float> mTotalRange;
        juce::Range<float> mVisibleRange;
    };
} // namespace OUS
