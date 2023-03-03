#include "Ruler.h"

using namespace OUS;

Ruler::Ruler(int tickPowerInteval, int tickDivisionFactor)
: mTickPowerInterval(tickPowerInteval)
, mTickDivisionFactor(tickDivisionFactor)
{
    
}

void Ruler::setSampleRate(double sampleRate)
{
    mSampleRate = sampleRate;
}

void Ruler::setTotalRange(juce::Range<float> totalRange)
{
    mTotalRange = totalRange;
    repaint();
}

void Ruler::setVisibleRange(juce::Range<float> visibleRange)
{
    mVisibleRange = visibleRange;
    repaint();
}

void Ruler::paint(juce::Graphics& g)
{
    
    g.fillAll(juce::Colours::black.withAlpha(0.4f));
    if(mVisibleRange.getLength() == 0)
    {
        return;
    }

    auto const bounds = getLocalBounds();
    auto const width = bounds.getWidth();
    auto const visibleRangeSamples = mVisibleRange.getLength() * mSampleRate;
    auto const sizeRatio = width / visibleRangeSamples;
    
    auto const font = g.getCurrentFont();
    auto const maxStringWidth = font.getStringWidthFloat(juce::String(static_cast<int>(mVisibleRange.getEnd() * mSampleRate))) + 8.0f;
    auto const minimumSampleSpacing = std::ceil(visibleRangeSamples * maxStringWidth / width);
    auto roundedSampleSpacingNonDivided = static_cast<int>(minimumSampleSpacing);
    
    auto const power = std::ceil(std::log(roundedSampleSpacingNonDivided)/std::log(mTickPowerInterval));
    roundedSampleSpacingNonDivided = static_cast<int>(std::pow(mTickPowerInterval, power));
    
    auto roundedSampleSpacing = roundedSampleSpacingNonDivided;
    if(mTickDivisionFactor > 1)
    {
        while(roundedSampleSpacing / mTickDivisionFactor *  sizeRatio > maxStringWidth)
        {
            roundedSampleSpacing /= mTickDivisionFactor;
        }
    }
    
    auto const labelWidth = roundedSampleSpacing / visibleRangeSamples * width;
    auto const numTicks = std::max(1, static_cast<int>(width / labelWidth));
    auto const visibleSampleRangeStart = mVisibleRange.getStart() * mSampleRate;
    auto const startSample = std::floor(visibleSampleRangeStart / roundedSampleSpacing) * roundedSampleSpacing;
    
    g.setColour(juce::Colours::white);
    for(int i = 0; i <= numTicks; ++i)
    {
        // get values
        auto const currentValue = startSample + static_cast<double>(i) * roundedSampleSpacing;
        auto const position = std::round((currentValue - visibleSampleRangeStart) * sizeRatio);
        g.drawLine(static_cast<float>(position), bounds.getY(), static_cast<float>(position), bounds.getY() + 8.0f);
        g.drawSingleLineText (juce::String(static_cast<int>(currentValue)), static_cast<int>(position), bounds.getBottom(), juce::Justification::left);
    }
}
