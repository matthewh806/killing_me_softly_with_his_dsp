#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../dsp/processors/TemplateProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    //==============================================================================
    class MainComponent : public juce::AudioAppComponent
    {
    public:
        //==============================================================================
        MainComponent(juce::AudioDeviceManager& deviceManager);
        ~MainComponent();

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override;
        void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

        //==============================================================================
        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        //==============================================================================
        TemplateProcessor mTemplateProcessor;

        int mBlockSize;
        int mSampleRate;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
} // namespace OUS
