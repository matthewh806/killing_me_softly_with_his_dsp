#pragma once

#include <JuceHeader.h>
#include "Waveform.h"
#include "Ruler.h"

namespace OUS 
{
    class WaveformAndRuler
    : public juce::Component
    , public juce::FileDragAndDropTarget
    , private juce::ChangeListener
    {
    public:
        
        WaveformAndRuler(juce::AudioFormatManager& formatManager);
        ~WaveformAndRuler() override;
        
        //==============================================================================
        // TODO: find way to way to avoid this?
        WaveformComponent const& getWaveform() const;
        void addWaveformChangeListener(ChangeListener* listener);
        void removeWaveformChangeListener(ChangeListener* listener);
        
        //==============================================================================
        // forwarding methods
        juce::Range<float> const& getVisibleRange() const;
        juce::Range<float> const& getTotalRange() const;
        
        void setSampleRate(float sampleRate);
        
        void clear();
        void resetZoom();
        
        std::function<void(juce::String&)> onNewFileDropped = nullptr;
        
        void setThumbnailSource(juce::AudioSampleBuffer* audioSource);
        
        //==============================================================================
        // juce::Component
        void resized() override;
        void mouseWheelMove(juce::MouseEvent const& event, juce::MouseWheelDetails const& wheel) override;
        
        //==============================================================================
        // juce::FileDragAndDropTarget
        bool isInterestedInFileDrag(const StringArray& files) override;
        void filesDropped(const StringArray& files, int x, int y) override;
        
        //==============================================================================
        // juce::ChangeListener
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
    private:
        WaveformComponent mWaveform;
        Ruler mSampleRuler;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformAndRuler)
    };
}
