#pragma once

#include "JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"
//==============================================================================

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
    
private:
    //==============================================================================
    int mBlockSize;
    int mSampleRate;
    
    juce::ImageButton mTestButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
