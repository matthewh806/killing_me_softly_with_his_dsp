#pragma once

#include "JuceHeader.h"
#include "../../core/CircularBuffer.h"
#include "../../ui/CustomLookAndFeel.h"

//==============================================================================
class SimpleDelayProcessor  : public juce::AudioProcessor
{
public:
    // TODO: Make this private
    juce::AudioProcessorValueTreeState state;
    
    //==============================================================================
    SimpleDelayProcessor();

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    //==============================================================================
    void prepareToPlay (double, int) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;

    //==============================================================================
    // TODO: Do it properly - this is not thread safe...
    void setWetDryMix(float newValue);
    void setDelayTime(float newValue);
    void setFeedback(float newValue);
    
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override            { return new SimpleDelayPluginProcessorEditor (*this); }
    bool hasEditor() const override                          { return true; }
    const String getName() const override                    { return "SimpleDelay"; }
    bool acceptsMidi() const override                        { return false; }
    bool producesMidi() const override                       { return false; }
    double getTailLengthSeconds() const override             { return 0.0; }
    int getNumPrograms() override                            { return 1; }
    int getCurrentProgram() override                         { return 0; }
    void setCurrentProgram (int) override                    {}
    const String getProgramName (int) override               { return {}; }
    void changeProgramName (int, const String&) override     {}
    bool isVST2() const noexcept                             { return (wrapperType == wrapperType_VST); }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    class SimpleDelayPluginProcessorEditor
    : public juce::AudioProcessorEditor
    {
    public:
        
        SimpleDelayPluginProcessorEditor(SimpleDelayProcessor& owner)
        : juce::AudioProcessorEditor(owner)
        , mDelayTimeSlider("Delay Time", "s")
        , mWetDrySlider("Wet / Dry", "")
        , mFeedbackSlider("Feedback", "")
        , mDelayTimeAttachment(owner.state, "delaytime", mDelayTimeSlider)
        , mWetDryAttachment(owner.state, "wetdry", mWetDrySlider)
        , mFeedbackAttachment(owner.state, "feedback", mFeedbackSlider)
        {
            addAndMakeVisible(&mDelayTimeSlider);
            mDelayTimeSlider.mLabels.add({0.0f, "0.1s"});
            mDelayTimeSlider.mLabels.add({1.0f, "2s"});
            
            addAndMakeVisible(&mWetDrySlider);
            mWetDrySlider.mLabels.add({0.0f, "0"});
            mWetDrySlider.mLabels.add({1.0f, "1"});
            
            addAndMakeVisible(&mFeedbackSlider);
            mFeedbackSlider.mLabels.add({0.0f, "0%"});
            mFeedbackSlider.mLabels.add({1.0f, "100%"});
            
            setSize (600, 200);
        }
        
        ~SimpleDelayPluginProcessorEditor() override {}
        
        //==============================================================================
        void paint (juce::Graphics& g) override
        {
            // (Our component is opaque, so we must completely fill the background with a solid colour)
            g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            bounds.removeFromTop(20);
            auto const rotaryWidth = static_cast<int>(bounds.getWidth() * 0.30);
            auto const spacing = static_cast<int>((bounds.getWidth() - (rotaryWidth * 3)) / 2.0);
            mDelayTimeSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
            bounds.removeFromLeft(spacing);
            mWetDrySlider.setBounds(bounds.removeFromLeft(rotaryWidth));
            bounds.removeFromLeft(spacing);
            mFeedbackSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
        }
        
    private:
        RotarySliderWithLabels mDelayTimeSlider;
        RotarySliderWithLabels mWetDrySlider;
        RotarySliderWithLabels mFeedbackSlider;
        
        juce::AudioProcessorValueTreeState::SliderAttachment mDelayTimeAttachment;
        juce::AudioProcessorValueTreeState::SliderAttachment mWetDryAttachment;
        juce::AudioProcessorValueTreeState::SliderAttachment mFeedbackAttachment;
    };
    
    //==============================================================================
    AudioParameterBool* mSyncMode;
    AudioParameterFloat* mWetDryMix;
    AudioParameterFloat* mDelayTime;
    AudioParameterFloat* mFeedback;
    
    int mBlockSize;
    int mSampleRate;
    
    CircularBuffer<float>** mDelayBuffers;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDelayProcessor)
};

