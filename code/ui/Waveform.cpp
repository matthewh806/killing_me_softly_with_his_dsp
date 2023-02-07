#include "Waveform.h"

using namespace OUS;

WaveformComponent::WaveformComponent(juce::AudioFormatManager& formatManager)
: mAudioFormatManager(formatManager)
, mThumbnailCache(32)
, mThumbnail(32, mAudioFormatManager, mThumbnailCache)
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
    mThumbnailBounds = getLocalBounds();
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
        g.setColour(juce::Colours::orange);
        mThumbnail.drawChannels(g, mThumbnailBounds, 0.0, mThumbnail.getTotalLength(), 1.0f);
    }

    if(!mThumbnail.isFullyLoaded())
    {
        juce::Component::SafePointer<juce::Component> sp{this};
        juce::MessageManager::callAsync([sp]()
                                        {
                                            if(auto c = sp.getComponent())
                                            {
                                                c->repaint();
                                            }
                                        });
    }
}

bool WaveformComponent::isInterestedInFileDrag(const StringArray& files)
{
    for(auto fileName : files)
    {
        if(!fileName.endsWith(".wav") && !fileName.endsWith(".aif") && !fileName.endsWith(".aiff"))
            return false;
    }

    return true;
}

void WaveformComponent::filesDropped(const StringArray& files, int x, int y)
{
    // only deal with one file for now.
    juce::ignoreUnused(x, y);

    auto const filePath = files[0];
    juce::File f{filePath};

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
