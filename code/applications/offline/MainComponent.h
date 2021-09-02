#pragma once

#include "JuceHeader.h"
#include "../../dsp/processors/RealTimeStretchProcessor.h"
#include "../../core/RingBuffer.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent, juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent(juce::AudioDeviceManager& audioDeviceManager);
    ~MainComponent() override;
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    void stretchValueChanged();
    void pitchShiftValueChanged();

    void performOfflineStretch();

    void changeState(TransportState newState);

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    //==============================================================================
    juce::TextButton mOpenButton;
    juce::TextButton mPlayButton;
    juce::TextButton mStopButton;

    RotarySliderWithLabels mStretchFactorSlider;
    RotarySliderWithLabels mPitchShiftSlider;

    juce::AudioFormatManager mFormatManager;
    juce::AudioTransportSource mTransportSource;
    juce::AudioSampleBuffer mFileBuffer;
    juce::AudioSampleBuffer mStretchedBuffer;

    std::unique_ptr<juce::MemoryAudioSource> mStretchedSrc;
    std::unique_ptr<juce::AudioFormatReaderSource> mReaderSource;
    std::unique_ptr<RubberBand::RubberBandStretcher> mRubberBandStretcher;

    TransportState mState;

    int mBlockSize;
    int mSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
