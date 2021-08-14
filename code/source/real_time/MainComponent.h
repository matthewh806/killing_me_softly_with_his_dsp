#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.

#include "JuceHeader.h"
#include "RubberBandStretcher.h"
#include "../RingBuffer.h"

//==============================================================================
class MainComponent   : public juce::AudioAppComponent, juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();
    
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
    
    void changeState(TransportState newState);
    
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    
    //==============================================================================
    juce::TextButton mOpenButton;
    juce::TextButton mPlayButton;
    juce::TextButton mStopButton;
    
    juce::Label mStretchFactorLabel;
    juce::Slider mStretchFactorSlider;
    
    juce::Label mPitchShiftLabel;
    juce::Slider mPitchShiftSlider;
    
    juce::AudioFormatManager mFormatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> mReaderSource;
    juce::AudioTransportSource mTransportSource;
    
    TransportState mState;
    
    std::unique_ptr<RubberBand::RubberBandStretcher> mRubberBand;
    
    juce::AudioSampleBuffer mInput;
    juce::AudioSampleBuffer mScratchBuffer;
    RubberBand::RingBuffer<float>** mOutputBuffer;
    
    int mBlockSize;
    int mSampleRate;
    
    int mMinfill = 0;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
