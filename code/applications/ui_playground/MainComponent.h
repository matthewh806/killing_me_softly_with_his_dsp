#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../ui/PixelArtLookAndFeel.h"
//==============================================================================

namespace OUS
{
    class MainComponent
    : public juce::AudioAppComponent
    {
    public:
        //==============================================================================
        MainComponent(juce::AudioDeviceManager& deviceManager);
        ~MainComponent() override;

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override;
        void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

        //==============================================================================
        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        //==============================================================================
        int mBlockSize;
        int mSampleRate;
        
        UI::PixelArt::TitleLabel mTitleLabel;
        UI::PixelArt::SyncButton mSyncButton;
        UI::PixelArt::PlayButton mPlayButton;
        UI::PixelArt::PauseButton mPauseButton;
        UI::PixelArt::RecordButton mRecordButton;
        UI::PixelArt::SelectorComponent mSelectorComponent;
        UI::PixelArt::RotarySliderWithLabels mRotarySlider{"Delay Time", "ms", 20.0f};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
} // namespace OUS
