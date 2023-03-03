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
        /*
         Use the tickPowerInterval to set the spacing of the tick labels
         in terms of a power series (e.g. 0, 512, 1024, 1536, 2048)
         
         The tick division factor will be used to further subdivide the
         tick spacing if there is room after the power series is determined
         */
        Ruler(int tickPowerInteval = 2, int tickDivisionFactor = 2);
        ~Ruler() override = default;
        
        void setSampleRate(double sampleRate);
        void setTotalRange(juce::Range<float> totalRange);
        void setVisibleRange(juce::Range<float> visibleRange);
        
        void paint(juce::Graphics& g) override;
    private:
        double mSampleRate;
        
        int mTickPowerInterval {10};
        int mTickDivisionFactor {2};
        int mPrimaryTickInterval {2};
        
        juce::Range<float> mTotalRange;
        juce::Range<float> mVisibleRange;
    };
} // namespace OUS
