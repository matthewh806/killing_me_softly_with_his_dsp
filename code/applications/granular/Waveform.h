#pragma once

#include <JuceHeader.h>

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
    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;
    
    // juce::AsyncUpdater
    void handleAsyncUpdate() override;
    
    std::function<void(juce::String&)> onNewFileDropped = nullptr;
    
private:
    juce::Rectangle<int> mThumbnailBounds;
    
    juce::AudioFormatManager& mAudioFormatManager;
    juce::AudioThumbnailCache mThumbnailCache;
    juce::AudioThumbnail mThumbnail;
    
    int64_t mStartSample = 0;
    int64_t mEndSample = 0;
    
    double mSampleRate = 44100.0;
};
