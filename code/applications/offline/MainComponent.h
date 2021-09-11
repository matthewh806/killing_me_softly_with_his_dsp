#pragma once

#include "JuceHeader.h"
#include "../../dsp/OfflineStretcher.h"
#include "../../core/RingBuffer.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent, juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
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
    
    void stretchButtonClicked();

    void performOfflineStretch();

    void changeState(TransportState newState);

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

    //==============================================================================
    juce::TextButton mOpenButton;
    juce::TextButton mPlayButton;
    juce::TextButton mStopButton;

    RotarySliderWithLabels mStretchFactorSlider;
    RotarySliderWithLabels mPitchShiftSlider;
    
    juce::TextButton mStretchButton;

    std::unique_ptr<juce::AudioFormatReaderSource> mReaderSource;
    
    juce::AudioFormatManager mFormatManager;
    juce::AudioTransportSource mTransportSource;
    juce::AudioSampleBuffer mFileBuffer;
    
    juce::TemporaryFile mStretchedFile {".wav"};
    
    TransportState mState;

    int mBlockSize;
    int mSampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
