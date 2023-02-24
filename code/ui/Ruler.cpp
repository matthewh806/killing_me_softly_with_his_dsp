#include "Ruler.h"

using namespace OUS;

Ruler::Ruler()
{
    
}

void Ruler::setSampleRate(double sampleRate)
{
    mSampleRate = sampleRate;
}

void Ruler::setTotalRange(juce::Range<float> totalRange)
{
    mTotalRange = totalRange;
}

void Ruler::setVisibleRange(juce::Range<float> visibleRange)
{
    mVisibleRange = visibleRange;
}

void Ruler::paint(juce::Graphics& g)
{
    
    g.fillAll(juce::Colours::black);
    if(mVisibleRange.getLength() == 0)
    {
        return;
    }

    auto const bounds = getLocalBounds();
    auto const width = bounds.getWidth();
    auto const visibleRangeSamples = mVisibleRange.getLength() * mSampleRate;
    auto constexpr maxStringWidth = 60.0f;
//    auto const maxTicks = width / maxStringWidth;
    auto const minimalTickSpacing = std::ceil(visibleRangeSamples * maxStringWidth / width);
    auto const labelWidth = minimalTickSpacing / visibleRangeSamples * width;
    auto const numTicks = static_cast<int>(width / labelWidth);
    auto const tickSpacingSamples = static_cast<int>(visibleRangeSamples / numTicks);
    auto const tickSpacing = width / numTicks;
    
    g.setColour(juce::Colours::white);
    for(int i = 0; i < numTicks; ++i)
    {
        // get value
        auto const pos = static_cast<double>(i) * tickSpacing;
        auto const tickValue = i * tickSpacingSamples;
        g.drawLine(pos, bounds.getY(), pos, bounds.getY() + 10.0f);
        g.drawSingleLineText (juce::String(tickValue), pos, bounds.getBottom(), juce::Justification::left);
    }
}
