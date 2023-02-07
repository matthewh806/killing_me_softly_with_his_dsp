#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../dsp/processors/RealTimeStretchProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    //==============================================================================
    class MainComponent : public juce::AudioAppComponent
    {
    public:
        //==============================================================================
        MainComponent(juce::AudioDeviceManager& audioDeviceManager);
        ~MainComponent() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override;
        void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

        //==============================================================================
        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        void stretchValueChanged();
        void pitchShiftValueChanged();

        //==============================================================================

        RotarySliderWithLabels mStretchFactorSlider;
        RotarySliderWithLabels mPitchShiftSlider;

        RealTimeStretchProcessor mStretchProcessor;

        int mBlockSize;
        int mSampleRate;

        int mMinfill = 0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
} // namespace OUS
