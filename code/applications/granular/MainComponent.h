#pragma once

#include "JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"
#include "../../core/ReferenceCountedBuffer.h"
#include "../../dsp/synthesis/granular/Scheduler.h"

#include "Waveform.h"


//==============================================================================

// This is the granulator class
class MainComponent
: public juce::AudioAppComponent
, private juce::Thread
, private juce::Timer
{
public:
    //==============================================================================
    MainComponent(juce::AudioDeviceManager& activeDeviceManager);
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
    
    void timerCallback() override;
    
    int mBlockSize;
    int mSampleRate;
    
    RotarySliderWithLabels mGrainDensitySlider;
    RotarySliderWithLabels mGrainLengthSlider;
    RotarySliderWithLabels mGrainPositionRandomnessSlider;
    NumberFieldWithLabel mGrainCountLabel;
    ComboBoxWithLabel mSourceTypeSlider;
    ComboBoxWithLabel mEnvelopeTypeSlider;
    
    juce::AudioFormatManager mFormatManager;
    std::unique_ptr<juce::AudioFormatReader> mReader;
    std::unique_ptr<Scheduler> mScheduler;
    
    WaveformComponent mWaveformComponent {mFormatManager};
    
    RotarySliderWithLabels mFrequencySlider;
    
    juce::SpinLock mMutex;
    juce::ReferenceCountedArray<ReferenceCountedBuffer> mBuffers;
    ReferenceCountedBuffer::Ptr mCurrentBuffer;
    
    Source::SourceType mSourceType {Source::SourceType::sample};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
