#pragma once

#include <JuceHeader.h>

namespace OUS
{
    class WaveformComponent
    : public juce::Component
    , public juce::FileDragAndDropTarget
    , private juce::AsyncUpdater
    {
    public:
        WaveformComponent(juce::AudioFormatManager& formatManager);
        ~WaveformComponent() override;

        juce::AudioThumbnail& getThumbnail();
        juce::Rectangle<int> const& getThumbnailBounds() const;

        void clear();

        // juce::Component
        void resized() override;
        void paint(juce::Graphics& g) override;

        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

        // juce::AsyncUpdater
        void handleAsyncUpdate() override;

        std::function<void(juce::String&)> onNewFileDropped = nullptr;

    private:
        juce::Rectangle<int> mThumbnailBounds;

        juce::AudioFormatManager& mAudioFormatManager;
        juce::AudioThumbnailCache mThumbnailCache;
        juce::AudioThumbnail mThumbnail;
    };
} // namespace OUS
