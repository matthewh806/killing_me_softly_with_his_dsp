#include "Waveform.h"

WaveformComponent::WaveformComponent(juce::AudioFormatManager& formatManager)
: mAudioFormatManager(formatManager)
, mThumbnailCache(1)
, mThumbnail(512, mAudioFormatManager, mThumbnailCache)
{
}

WaveformComponent::~WaveformComponent()
{
    
}


juce::AudioThumbnail& WaveformComponent::getThumbnail()
{
    return mThumbnail;
}

juce::Rectangle<int> const& WaveformComponent::getThumbnailBounds() const
{
    return mThumbnailBounds;
}

void WaveformComponent::clear()
{
    mThumbnailCache.clear();
    mThumbnail.clear();
}

void WaveformComponent::resized()
{
    mThumbnailBounds = juce::Rectangle<int>(10, 10, getWidth()-20, getHeight()-20);
}

void WaveformComponent::paint(juce::Graphics& g)
{
    if(mThumbnail.getNumChannels() == 0)
    {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(mThumbnailBounds);
        g.setColour(juce::Colours::white);
        g.drawFittedText("Drag and drop and audio file", mThumbnailBounds, juce::Justification::centred, 1);
    }
    else
    {
        g.setColour(juce::Colours::white);
        g.fillRect(mThumbnailBounds);
        g.setColour(juce::Colours::red);
        mThumbnail.drawChannels(g, mThumbnailBounds, 0.0, mThumbnail.getTotalLength(), 1.0f);
    }
    
    juce::Range<int64_t> sampleRange { mStartSample, mEndSample };
    if(sampleRange.getLength() == 0)
    {
        return;
    }
    
    auto const sampleStartRatio = static_cast<double>(sampleRange.getStart() / mSampleRate) / mThumbnail.getTotalLength();
    auto const sampleSizeRatio = static_cast<double>(sampleRange.getLength() / mSampleRate) / mThumbnail.getTotalLength();
    
    juce::Rectangle<int> clipBounds {
        mThumbnailBounds.getX() + static_cast<int>(mThumbnailBounds.getWidth() * sampleStartRatio),
        mThumbnailBounds.getY(),
        static_cast<int>(mThumbnailBounds.getWidth() * sampleSizeRatio),
        mThumbnailBounds.getHeight()
    };
    
    g.setColour(juce::Colours::blue.withAlpha(0.4f));
    g.fillRect(clipBounds);
}

bool WaveformComponent::isInterestedInFileDrag (const StringArray& files)
{
    for(auto fileName : files)
    {
        if(!fileName.endsWith(".wav") && !fileName.endsWith(".aif") && !fileName.endsWith(".aiff"))
            return false;
    }
    
    return true;
}

void WaveformComponent::filesDropped (const StringArray& files, int x, int y)
{
    // only deal with one file for now.
    juce::ignoreUnused(x, y);
    
    auto const filePath = files[0];
    juce::File f { filePath };
    
    if(f.existsAsFile())
    {
        auto path = f.getFullPathName();
        
        if(onNewFileDropped != nullptr)
        {
            onNewFileDropped(path);
        }
    }
}

void WaveformComponent::handleAsyncUpdate()
{
    repaint();
}
