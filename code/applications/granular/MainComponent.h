#pragma once

#include "JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"
#include "../../core/ReferenceCountedBuffer.h"

#include "Waveform.h"
#include "Scheduler.h"

//==============================================================================

// This is the granulator class
class MainComponent
: public juce::AudioAppComponent
, private juce::Thread
{
public:
    //==============================================================================
    MainComponent(juce::AudioDeviceManager& deviceManager);
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool loadSample(juce::String const& filePath, juce::String& error);
    
private:
    //==============================================================================
    void run() override;
    void checkForBuffersToFree();
    
    int mBlockSize;
    int mSampleRate;
    
    RotarySliderWithLabels mGrainDensity;
    RotarySliderWithLabels mGrainLength;
    
    juce::AudioFormatManager mFormatManager;
    std::unique_ptr<juce::AudioFormatReader> mReader;
    std::unique_ptr<Scheduler> mScheduler;
    
    WaveformComponent mWaveformComponent {*this, mFormatManager};
    
    juce::SpinLock mMutex;
    juce::ReferenceCountedArray<ReferenceCountedBuffer> mBuffers;
    ReferenceCountedBuffer::Ptr mCurrentBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
