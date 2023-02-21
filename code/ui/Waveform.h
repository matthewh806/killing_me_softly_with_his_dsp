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
        
        void setThumbnailSource(juce::AudioSampleBuffer* audioSource);
        void clear();

        // juce::Component
        void resized() override;
        void paint(juce::Graphics& g) override;
        
        // TODO: Add double click behaviour to restore to default zoom?
        
        void mouseWheelMove(juce::MouseEvent const& event, juce::MouseWheelDetails const& wheel) override;

        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;

        // juce::AsyncUpdater
        void handleAsyncUpdate() override;

        std::function<void(juce::String&)> onNewFileDropped = nullptr;

    private:
        void updateWaveformZoom(float deltaY, float anchorPoint);
        
        juce::Rectangle<int> mThumbnailBounds;
        
        // Samples / times / Pixels???
        // thumbnail draw method works in terms of seconds
        juce::Range<float> mTotalRange;
        juce::Range<float> mVisibleRange;

        juce::AudioFormatManager& mAudioFormatManager;
        juce::AudioThumbnailCache mThumbnailCache;
        juce::AudioThumbnail mThumbnail;
        
        float mSampleRate = 44100.0;
        
        bool mIsMovingMouseWheel = false;
        float mTotalWheelDisplacemet = 0.0f;
        float mZoomAnchor = 0.0f;
    };
} // namespace OUS
