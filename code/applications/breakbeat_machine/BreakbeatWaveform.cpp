#include "BreakbeatWaveform.h"

using namespace OUS;

PlayheadPositionOverlayComponent::PlayheadPositionOverlayComponent(juce::AudioTransportSource& transportSource)
: mTransportSource(transportSource)
{
    startTimer(40);
}

PlayheadPositionOverlayComponent::~PlayheadPositionOverlayComponent()
{
}

void PlayheadPositionOverlayComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    auto const audioLength = mTransportSource.getLengthInSeconds();

    if(audioLength <= 0.0)
    {
        return;
    }

    auto const playheadDrawPosition = static_cast<float>(mPlayheadPosition / audioLength * getWidth());
    g.drawLine(playheadDrawPosition, 0.0f, playheadDrawPosition, getHeight(), 2.0f);
}

void PlayheadPositionOverlayComponent::timerCallback()
{
    auto const playheadPosition = static_cast<float>(mTransportSource.getCurrentPosition());
    if(std::abs(mPlayheadPosition - playheadPosition) > std::numeric_limits<float>::epsilon())
    {
        mPlayheadPosition = playheadPosition;
        repaint();
    }
}

SlicesOverlayComponent::SlicesOverlayComponent(juce::AudioTransportSource& transportSource)
: mTransportSource(transportSource)
{
}

SlicesOverlayComponent::~SlicesOverlayComponent()
{
}

void SlicesOverlayComponent::paint(juce::Graphics& g)
{
    if(mSlicePositions.size() <= 1)
    {
        return;
    }

    // paint all of the slices
    g.setColour(juce::Colours::black);

    auto const audioLength = mTransportSource.getLengthInSeconds();
    for(size_t i = 0; i < mSlicePositions.size(); ++i)
    {
        auto const startRatio = static_cast<double>(mSlicePositions[i] / mSampleRate) / audioLength;
        auto const sliceX = static_cast<int>(getWidth() * startRatio);
        g.drawVerticalLine(sliceX, 0.0f, getBottom());

        // draw the slice handle
        Path trianglePath;
        trianglePath.addTriangle(sliceX - 8.0f, 0.0f, sliceX + 8.0f, 0.0f, sliceX, 16.0f);
        g.fillPath(trianglePath);
    }

    // paint active slice range;
    auto const activeSliceStart = mSlicePositions[mActiveSliceIndex];
    auto const activeSliceEnd = mActiveSliceIndex >= mSlicePositions.size() ? static_cast<size_t>(audioLength * mSampleRate) : mSlicePositions[mActiveSliceIndex + 1];

    juce::Range<size_t> sampleRange{activeSliceStart, activeSliceEnd};
    if(sampleRange.getLength() == 0)
    {
        return;
    }

    auto const sampleStartRatio = static_cast<double>(sampleRange.getStart() / mSampleRate) / audioLength;
    auto const sampleSizeRatio = static_cast<double>(sampleRange.getLength() / mSampleRate) / audioLength;

    juce::Rectangle<int> clipBounds{static_cast<int>(getWidth() * sampleStartRatio), getY(), static_cast<int>(getWidth() * sampleSizeRatio), getHeight()};

    g.setColour(juce::Colours::blue.withAlpha(0.4f));
    g.fillRect(clipBounds);
}

void SlicesOverlayComponent::setSlicePositions(std::vector<SliceManager::Slice> const& slices, size_t activeSliceIndex)
{
    mSlicePositions.clear();
    mSlicePositions.resize(slices.size());

    for(size_t i = 0; i < mSlicePositions.size(); ++i)
    {
        mSlicePositions[i] = std::get<1>(slices[i]);
    }

    mActiveSliceIndex = activeSliceIndex;

    repaint();
}

void SlicesOverlayComponent::setActiveSlice(size_t sliceIndex)
{
    mActiveSliceIndex = sliceIndex;
    repaint();
}

void SlicesOverlayComponent::setSampleRate(float sampleRate)
{
    mSampleRate = sampleRate;
}

std::optional<int> SlicesOverlayComponent::getSliceUnderMousePosition(int x, int y)
{
    if(y > 20)
    {
        return {};
    }

    // check all slices to check if its in the triangle bounds
    auto outSliceX = -1;

    auto const audioLength = mTransportSource.getLengthInSeconds();
    for(size_t i = 0; i < mSlicePositions.size(); ++i)
    {
        auto const startRatio = static_cast<double>(mSlicePositions[i] / mSampleRate) / audioLength;
        auto const sliceX = static_cast<int>(getWidth() * startRatio);

        if(x > sliceX - 8 && x < sliceX + 8)
        {
            outSliceX = sliceX;
            break;
        }
    }

    if(outSliceX < 0)
    {
        return {};
    }

    return outSliceX;
}

BreakbeatWaveformComponent::BreakbeatWaveformComponent(juce::AudioFormatManager& formatManager, juce::AudioTransportSource& transportSource)
: mAudioFormatManager(formatManager)
, mWaveformComponent(formatManager)
, mSliceOverlayComponent(transportSource)
, mPlayheadOverlayComponent(transportSource)
{
    mWaveformComponent.getThumbnail().addChangeListener(this);

    addAndMakeVisible(mWaveformComponent);
    addAndMakeVisible(mSliceOverlayComponent);
    addAndMakeVisible(mPlayheadOverlayComponent);

    mPlayheadOverlayComponent.setInterceptsMouseClicks(false, false);
    mSliceOverlayComponent.setInterceptsMouseClicks(false, false);
    mWaveformComponent.setInterceptsMouseClicks(false, false);

    mWaveformComponent.onNewFileDropped = [this](juce::String& path)
    {
        if(onNewFileDropped != nullptr)
        {
            onNewFileDropped(path);
        }
    };
}

BreakbeatWaveformComponent::~BreakbeatWaveformComponent()
{
    mWaveformComponent.getThumbnail().removeChangeListener(this);
}

juce::AudioThumbnail& BreakbeatWaveformComponent::getThumbnail()
{
    return mWaveformComponent.getThumbnail();
}

void BreakbeatWaveformComponent::setSlicePositions(std::vector<SliceManager::Slice> const& slicePositions, size_t activeSliceIndex)
{
    mSliceOverlayComponent.setSlicePositions(slicePositions, activeSliceIndex);
}

void BreakbeatWaveformComponent::setActiveSlice(size_t sliceIndex)
{
    mSliceOverlayComponent.setActiveSlice(sliceIndex);
}

void BreakbeatWaveformComponent::clear()
{
    mWaveformComponent.clear();
}

void BreakbeatWaveformComponent::setSampleRate(float sampleRate)
{
    mSliceOverlayComponent.setSampleRate(sampleRate);
}

void BreakbeatWaveformComponent::resized()
{
    auto componentBounds = getLocalBounds();
    mSliceOverlayComponent.setBounds(componentBounds);

    // This reduction is done to give the slice markers some space at the top
    componentBounds.removeFromTop(18.0);
    mWaveformComponent.setBounds(componentBounds);
    mPlayheadOverlayComponent.setBounds(componentBounds);
}

void BreakbeatWaveformComponent::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

void BreakbeatWaveformComponent::mouseDoubleClick(juce::MouseEvent const& event)
{
    if(onWaveformDoubleClicked != nullptr)
    {
        onWaveformDoubleClicked(event.x);
    }
}

void BreakbeatWaveformComponent::mouseDown(juce::MouseEvent const& event)
{
    if(event.y > 20)
    {
        return;
    }

    if(onSliceMarkerMouseDown != nullptr)
    {
        auto slice = mSliceOverlayComponent.getSliceUnderMousePosition(event.x, event.y);
        if(slice.has_value())
        {
            onSliceMarkerMouseDown(slice.value());
        }
    }
}

void BreakbeatWaveformComponent::mouseUp(juce::MouseEvent const& event)
{
    if(event.mods.isRightButtonDown())
    {
        if(onMouseUp != nullptr)
        {
            auto slice = mSliceOverlayComponent.getSliceUnderMousePosition(event.x, event.y);
            if(slice.has_value())
            {
                onSliceMarkerRightClicked(slice.value());
            }
        }
    }

    if(onMouseUp != nullptr)
    {
        onMouseUp();
    }
}

void BreakbeatWaveformComponent::mouseDrag(const MouseEvent& event)
{
    // increment position of the marker
    if(onSliceMarkerDragged != nullptr)
    {
        onSliceMarkerDragged(event.position.x);
    }
}

void BreakbeatWaveformComponent::handleAsyncUpdate()
{
    repaint();
}

void BreakbeatWaveformComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if(source == &mWaveformComponent.getThumbnail())
    {
        repaint();
    }
}

// juce::FileDragAndDropTarget
bool BreakbeatWaveformComponent::isInterestedInFileDrag(const StringArray& files)
{
    return mWaveformComponent.isInterestedInFileDrag(files);
}

void BreakbeatWaveformComponent::filesDropped(const StringArray& files, int x, int y)
{
    mWaveformComponent.filesDropped(files, x, y);
}
