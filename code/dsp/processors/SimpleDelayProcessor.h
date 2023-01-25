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
    constexpr static float syncedDelayDivisions[7] = {1, 2, 4, 8, 16, 32, 64};
    
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
    , private juce::AudioProcessorValueTreeState::Listener
    {
    public:
        
        SimpleDelayPluginProcessorEditor(SimpleDelayProcessor& owner)
        : juce::AudioProcessorEditor(owner)
        , mSyncToggle("Sync")
        , mDelayTimeSlider("Delay Time", "s")
        , mSyncedDelaySlider("Beat Fraction", "")
        , mWetDrySlider("Wet / Dry", "")
        , mFeedbackSlider("Feedback", "")
        , mSyncToggleAttachment(owner.state, "sync", mSyncToggle)
        , mDelayTimeAttachment(owner.state, "delaytime", mDelayTimeSlider)
        , mSyncedDelayTimeAttachment(owner.state, "delaydivisor", mSyncedDelaySlider)
        , mWetDryAttachment(owner.state, "wetdry", mWetDrySlider)
        , mFeedbackAttachment(owner.state, "feedback", mFeedbackSlider)
        {
            addAndMakeVisible(mSyncToggle);
            owner.state.addParameterListener("sync", this);
            
            addAndMakeVisible(&mDelayTimeSlider);
            mDelayTimeSlider.mLabels.add({0.0f, "0.1s"});
            mDelayTimeSlider.mLabels.add({1.0f, "2s"});
            
            addAndMakeVisible(&mSyncedDelaySlider);
            mSyncedDelaySlider.mLabels.add({0.0f, "1"});
            mSyncedDelaySlider.mLabels.add({1.0f, "1/64"});
            
            addAndMakeVisible(&mWetDrySlider);
            mWetDrySlider.mLabels.add({0.0f, "0"});
            mWetDrySlider.mLabels.add({1.0f, "1"});
            
            addAndMakeVisible(&mFeedbackSlider);
            mFeedbackSlider.mLabels.add({0.0f, "0%"});
            mFeedbackSlider.mLabels.add({1.0f, "100%"});
            
            mDelayTimeSlider.setVisible(true);
            mSyncedDelaySlider.setVisible(false);
            
            setSize (600, 250);
        }
        
        ~SimpleDelayPluginProcessorEditor() override
        {
            dynamic_cast<SimpleDelayProcessor&>(processor).state.removeParameterListener("sync", this);
        }
        
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
            
            auto syncButtonBounds = bounds.removeFromTop(20);
            mSyncToggle.setBounds(syncButtonBounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.30)));
            
            auto const rotaryWidth = static_cast<int>(bounds.getWidth() * 0.30);
            auto const spacing = static_cast<int>((bounds.getWidth() - (rotaryWidth * 3)) / 2.0);
            
            auto delaySliderBounds = bounds.removeFromLeft(rotaryWidth);
            mDelayTimeSlider.setBounds(delaySliderBounds);
            mSyncedDelaySlider.setBounds(delaySliderBounds);
            
            bounds.removeFromLeft(spacing);
            mWetDrySlider.setBounds(bounds.removeFromLeft(rotaryWidth));
            bounds.removeFromLeft(spacing);
            mFeedbackSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
        }
        
    private:
        
        class SyncedDelayRotarySliderWithLabels : public RotarySliderWithLabels
        {
        public:
            using RotarySliderWithLabels::RotarySliderWithLabels;
            
            juce::String getDisplayString() const override
            {
                auto const strVal = juce::String(syncedDelayDivisions[static_cast<int>(getValue())]);
                return "1 / " + strVal;
            }
        };
        
        // juce::AudioProcessorValueTreeState::Listener
        void parameterChanged (const String& parameterID, float newValue) override
        {
            if(parameterID.equalsIgnoreCase("sync"))
            {
                auto const synced = newValue >= 0.5f;
                mDelayTimeSlider.setVisible(!synced);
                mSyncedDelaySlider.setVisible(synced);
                
                repaint();
            }
        }
        
        juce::ToggleButton mSyncToggle;
        
        RotarySliderWithLabels mDelayTimeSlider;
        SyncedDelayRotarySliderWithLabels mSyncedDelaySlider;
        
        RotarySliderWithLabels mWetDrySlider;
        RotarySliderWithLabels mFeedbackSlider;
        
        juce::AudioProcessorValueTreeState::ButtonAttachment mSyncToggleAttachment;
        juce::AudioProcessorValueTreeState::SliderAttachment mDelayTimeAttachment;
        juce::AudioProcessorValueTreeState::SliderAttachment mSyncedDelayTimeAttachment;
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
    
    std::vector<std::unique_ptr<CircularBuffer<float>>> mDelayBuffers;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDelayProcessor)
};

