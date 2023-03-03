#include "Waveform.h"

using namespace OUS;

WaveformComponent::WaveformComponent(juce::AudioFormatManager& formatManager)
: mAudioFormatManager(formatManager)
, mThumbnailCache(512)
, mThumbnail(2048, mAudioFormatManager, mThumbnailCache)
{
}

WaveformComponent::~WaveformComponent()
{
}

juce::Rectangle<int> const& WaveformComponent::getThumbnailBounds() const
{
    return mThumbnailBounds;
}

juce::Range<float> const& WaveformComponent::getVisibleRange() const
{
    return mVisibleRange;
}

juce::Range<float> const& WaveformComponent::getTotalRange() const
{
    return mTotalRange;
}

void WaveformComponent::setZoomable(bool zoomable)
{
    mZoomable = zoomable;
}

void WaveformComponent::resetZoom()
{
    mVisibleRange = mTotalRange;
    
    sendChangeMessage();
    repaint();
}

void WaveformComponent::setThumbnailSource(juce::AudioSampleBuffer* audioSource)
{
    // TODO Check nullptr audiosource;
    clear();
    mThumbnail.setSource(audioSource, mSampleRate, 0);
    
    mMinimumLength = 1.0f / mSampleRate;
    mTotalRange = juce::Range<float>(0.0f, static_cast<float>(mThumbnail.getTotalLength()));
    mVisibleRange = mTotalRange;
    mZoomAnchor = mVisibleRange.getLength() / 2.0f;
    
    sendChangeMessage();
    repaint();
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
        mThumbnail.drawChannels(g, mThumbnailBounds, mVisibleRange.getStart(), mVisibleRange.getEnd(), 1.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.4f));
        
        // This works best if the component is an odd size since we can properly center around 0 point then
        auto const height = mThumbnailBounds.getHeight();
        auto const numChannels = mThumbnail.getNumChannels();
        
        if(numChannels == 1)
        {
            g.drawHorizontalLine(mThumbnailBounds.getX() + static_cast<int>(height * 0.5f), mThumbnailBounds.getX(), mThumbnailBounds.getRight());
        }
        else if(numChannels == 2)
        {
            g.drawHorizontalLine(mThumbnailBounds.getX() + static_cast<int>(height * 0.25f), mThumbnailBounds.getX(), mThumbnailBounds.getRight());
            g.drawHorizontalLine(mThumbnailBounds.getX() + static_cast<int>(height * 0.75f), mThumbnailBounds.getX(), mThumbnailBounds.getRight());
        }
        
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

void WaveformComponent::mouseWheelMove(juce::MouseEvent const& event, juce::MouseWheelDetails const& wheel)
{
    if(mThumbnail.getNumChannels() == 0)
    {
        return;
    }
    
    if(!mZoomable)
    {
        return;
    }
    
    // Force into translate onlym fixed visible range mode when alt is held
    auto const lockedZoomLevel = event.mods.isAltDown();
    if(!lockedZoomLevel)
    {
        updateWaveformZoom(wheel.deltaY, event.x);
    }
    
    updateWaveformPosition(wheel.deltaX, lockedZoomLevel);
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

void WaveformComponent::updateWaveformZoom(float deltaY, float anchorPoint)
{
    auto const startOfAction = !mIsMovingMouseWheel && deltaY != 0;
    mIsMovingMouseWheel = deltaY != 0.0;
    
    if(!mIsMovingMouseWheel)
    {
        mTotalWheelDisplacemet = 0.0f;
        return;
    }
    
    if(startOfAction)
    {
        // Convert anchor point from pixels to time
        auto const waveformWidth = mThumbnailBounds.getWidth();
        mZoomAnchor = static_cast<float>(((anchorPoint - mThumbnailBounds.getX()) * mVisibleRange.getLength()) / waveformWidth) + mVisibleRange.getStart();
//        std::cout << "New Anchor Pos: " << mZoomAnchor << "\n";
    }
    
    mTotalWheelDisplacemet += deltaY;
    auto const zoomInc = std::pow(1.05f, std::abs(mTotalWheelDisplacemet));
    auto const zoomFactor = mTotalWheelDisplacemet < 0.0f ? zoomInc : 1.0f / zoomInc;
    
    // Range at zoomFactor 1
    mVisibleRange = juce::Range<float>(mZoomAnchor - (mZoomAnchor - mVisibleRange.getStart()) * zoomFactor, mZoomAnchor + (mVisibleRange.getEnd() - mZoomAnchor) * zoomFactor);
    
    // Constrain to total rannge
    mVisibleRange = mVisibleRange.getIntersectionWith(mTotalRange);
    
    // ensure its bigger than the min length
    mVisibleRange.setEnd(std::max(mVisibleRange.getEnd(), mVisibleRange.getStart() + mMinimumLength));
    
//    std::cout << "zoomFactor: " << zoomFactor << ", visRange: " << mVisibleRange.getStart() << ", " << mVisibleRange.getEnd() << "\n";
    sendChangeMessage();
    repaint();
}

void WaveformComponent::updateWaveformPosition(float deltaX, bool lockZoomLevel)
{
    auto const waveformWidth = mThumbnailBounds.getWidth();
    
    // TODO: Experiment with setting this multiplication factor based on zoom level?
    auto const translation = -1.0f * deltaX * 10.0f / static_cast<float>(waveformWidth) * mVisibleRange.getLength();
    auto const newRange = mVisibleRange + translation;
    
    if(lockZoomLevel)
    {
        // visible range is fixed, translation happens at fixed zoom level and is bound by start / end
        auto const constrainedRange = juce::Range<float>{std::max(newRange.getStart(), mTotalRange.getStart()), std::min(newRange.getEnd(), mTotalRange.getEnd())};
        if(std::abs(mVisibleRange.getLength() - constrainedRange.getLength()) < std::numeric_limits<float>::epsilon())
        {
            mVisibleRange = constrainedRange;
        }
    }
    else
    {
        // reduce visible range possible when translating at the start / end
        mVisibleRange = newRange;
        mVisibleRange = mVisibleRange.getIntersectionWith(mTotalRange);
    }
    
    // Constrain min size
    mVisibleRange.setEnd(std::max(mVisibleRange.getEnd(), mVisibleRange.getStart() + mMinimumLength));
    
    sendChangeMessage();
    repaint();
}
