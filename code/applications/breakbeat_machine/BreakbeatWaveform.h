#pragma once

#include "../../ui/Waveform.h"
#include "SliceManager.h"
#include <JuceHeader.h>

namespace OUS
{
    class PlayheadPositionOverlayComponent
    : public juce::Component
    , private juce::Timer
    {
    public:
        PlayheadPositionOverlayComponent(juce::AudioTransportSource& transportSource);
        ~PlayheadPositionOverlayComponent() override;

        void paint(juce::Graphics& g) override;

    private:
        void timerCallback() override;

        juce::AudioTransportSource& mTransportSource;
        float mPlayheadPosition{0.0f};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayheadPositionOverlayComponent)
    };

    class SlicesOverlayComponent
    : public juce::Component
    {
    public:
        SlicesOverlayComponent(juce::AudioTransportSource& transportSource);
        ~SlicesOverlayComponent() override;

        void paint(juce::Graphics& g) override;

        void setSlicePositions(std::vector<SliceManager::Slice> const& slices, size_t activeSliceIndex);
        void setActiveSlice(size_t sliceIndex);

        void setSampleRate(float sampleRate);

        std::optional<int> getSliceUnderMousePosition(int x, int y);

    private:
        float mSampleRate{44100.0f};

        juce::AudioTransportSource& mTransportSource;

        std::vector<size_t> mSlicePositions;
        size_t mActiveSliceIndex{0};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlicesOverlayComponent)
    };

    class BreakbeatWaveformComponent
    : public juce::Component
    , public juce::FileDragAndDropTarget
    , private juce::AsyncUpdater
    , private juce::ChangeListener
    {
    public:
        BreakbeatWaveformComponent(juce::AudioFormatManager& formatManager, juce::AudioTransportSource& transportSource);
        ~BreakbeatWaveformComponent() override;

        juce::AudioThumbnail& getThumbnail();

        void setSlicePositions(std::vector<SliceManager::Slice> const& slicePositions, size_t activeSliceIndex);
        void setActiveSlice(size_t sliceIndex);

        void setSampleRate(float sampleRate);

        void clear();

        // juce::Component
        void resized() override;
        void paint(juce::Graphics& g) override;
        void mouseDoubleClick(juce::MouseEvent const& event) override;
        void mouseDown(juce::MouseEvent const& event) override;
        void mouseUp(juce::MouseEvent const& event) override;
        void mouseDrag(const MouseEvent& event) override;

        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

        // juce::AsyncUpdater
        void handleAsyncUpdate() override;

        // juce::ChangeListener
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        std::function<void(int)> onWaveformDoubleClicked = nullptr;
        std::function<void(int)> onSliceMarkerRightClicked = nullptr;
        std::function<void(int)> onSliceMarkerMouseDown = nullptr;
        std::function<void(float)> onSliceMarkerDragged = nullptr;
        std::function<void()> onMouseUp = nullptr;

        std::function<void(juce::String&)> onNewFileDropped = nullptr;

    private:
        juce::AudioFormatManager& mAudioFormatManager;

        WaveformComponent mWaveformComponent;
        SlicesOverlayComponent mSliceOverlayComponent;
        PlayheadPositionOverlayComponent mPlayheadOverlayComponent;

        double mSampleRate = 44100.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BreakbeatWaveformComponent)
    };
} // namespace OUS
