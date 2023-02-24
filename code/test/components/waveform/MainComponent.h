#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../../core/ReferenceCountedBuffer.h"
#include "../../../ui/CustomLookAndFeel.h"
#include "../../../ui/Waveform.h"
#include "../../../ui/Ruler.h"

namespace OUS
{
    // TODO: Rename and move to UI directory
    class WaveformComposite
    : public juce::Component
    {
    public:
        
        // make private, forwarding methods etc
        WaveformComponent mWaveform;
        Ruler mSampleRuler;
        
        WaveformComposite(juce::AudioFormatManager& formatManager);
        ~WaveformComposite() override = default;
        
        void setThumbnailSource(juce::AudioSampleBuffer* audioSource);
        
        //==============================================================================
        void resized() override;
        
    private:
        
    };

    //==============================================================================
    class WaveformComponentTest
    : public juce::Component
    , private juce::Thread
    {
    public:
        //==============================================================================
        WaveformComponentTest(juce::AudioDeviceManager& deviceManager);
        ~WaveformComponentTest() override = default;

        //==============================================================================
        void paint(juce::Graphics&) override;
        void resized() override;
        
        void mouseUp(juce::MouseEvent const& event) override;

    private:
        void newFileDropped(juce::String& filePath);
        
        void run() override;
        void checkForBuffersToFree();
        
        std::unique_ptr<juce::AudioFormatReader> mReader;
        juce::AudioFormatManager mAudioFormats;
        
        juce::SpinLock mMutex;
        juce::ReferenceCountedArray<ReferenceCountedBuffer> mBuffers;
        ReferenceCountedBuffer::Ptr mCurrentBuffer;
        
        WaveformComposite mWaveformComposite{mAudioFormats};
        
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponentTest)
    };
} // namespace OUS
