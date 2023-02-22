#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../../core/ReferenceCountedBuffer.h"
#include "../../../ui/CustomLookAndFeel.h"
#include "../../../ui/Waveform.h"

namespace OUS
{
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
        
        WaveformComponent mWaveform{mAudioFormats};
        
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponentTest)
    };
} // namespace OUS
