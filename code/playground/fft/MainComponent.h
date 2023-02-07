#pragma once

#include "JuceHeader.h"
#include "FFTWrapper.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
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
        //==============================================================================
        enum TransportState
        {
            Stopped,
            Starting,
            Playing,
            Stopping
        };
        
        void openButtonClicked();
        void playButtonClicked(juce::PositionableAudioSource* audioSource);
        void stopButtonClicked();
        
        void performFFT();
        
        void changeState(TransportState newState);
        void changeListenerCallback (juce::ChangeBroadcaster* source) override;
        
        int mBlockSize;
        int mSampleRate;
        
        juce::TextButton mOpenButton;
        
        std::unique_ptr<juce::AudioFormatReaderSource> mReaderSource;
        std::unique_ptr<juce::MemoryAudioSource> mReconstructedReaderSource;
        
        juce::TextButton mPlayButton;
        juce::TextButton mStopButton;
        juce::TextButton mFFTButton;
        
        juce::TextButton mPerformFFTButton;
        juce::TextButton mPlayReconstructedSoundButton;
        
        juce::AudioFormatManager mFormatManager;
        juce::AudioTransportSource mTransportSource;
        juce::AudioSampleBuffer mFileBuffer;
        
        TransportState mState;
        
        FFTWrapper mFFTWrapper;
        std::array<float, FFTWrapper::FFTSize * 2> mFFTData;
        size_t mFifoIndex = 0;
        size_t mOutputIndex = 0;
        bool mPerformFFT = false;
        bool mOutputAvailable = false;
        
        juce::AudioSampleBuffer mReconstructedAudioBuffer;
        
        std::unique_ptr<juce::FileChooser> mFileChooser = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
