#pragma once

#include "JuceHeader.h"
#include "../../dsp/processors/TemplateProcessor.h"
#include "../../ui/CustomLookAndFeel.h"

#include "Waveform.h"
#include "Scheduler.h"

//==============================================================================

// This is the granulator class
class MainComponent
: public juce::AudioAppComponent
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
    int mBlockSize;
    int mSampleRate;
    
    juce::AudioFormatManager mFormatManager;
    std::unique_ptr<juce::AudioFormatReader> mReader;
    std::unique_ptr<Scheduler> mScheduler;
    
    WaveformComponent mWaveformComponent {*this, mFormatManager};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
