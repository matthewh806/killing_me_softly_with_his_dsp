#pragma once

#include <JuceHeader.h>
#include "Waveform.h"
#include "Ruler.h"

namespace OUS 
{
    class WaveformAndRuler
    : public juce::Component
    , private juce::ChangeListener
    {
    public:
        
        WaveformAndRuler(juce::AudioFormatManager& formatManager);
        ~WaveformAndRuler() override;
        
        // forwarding method
        void resetZoom();
        std::function<void(juce::String&)> onNewFileDropped = nullptr;
        
        void setThumbnailSource(juce::AudioSampleBuffer* audioSource);
        
        //==============================================================================
        void resized() override;
        
        //==============================================================================
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
    private:
        WaveformComponent mWaveform;
        Ruler mSampleRuler;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformAndRuler)
    };
}
