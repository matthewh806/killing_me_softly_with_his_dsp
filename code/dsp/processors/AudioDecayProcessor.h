#pragma once

#include "JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"

namespace OUS
{
    //==============================================================================
    class AudioDecayProcessor
    : public juce::AudioProcessor
    , private juce::AudioProcessorValueTreeState::Listener
    {
    public:
        // TODO: Make this private
        juce::AudioProcessorValueTreeState state;
        
        //==============================================================================
        AudioDecayProcessor();

        //==============================================================================
        bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

        //==============================================================================
        void prepareToPlay (double, int) override;
        void releaseResources() override;
        void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override;
        
        //==============================================================================
        juce::AudioProcessorEditor* createEditor() override            { return new AudioDecayPluginProcessorEditor (*this); }
        bool hasEditor() const override                          { return true; }
        const String getName() const override                    { return "AudioDecay"; }
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
        class AudioDecayPluginProcessorEditor
        : public juce::AudioProcessorEditor
        {
        public:
            
            AudioDecayPluginProcessorEditor(AudioDecayProcessor& owner)
            : juce::AudioProcessorEditor(owner)
            , mBitDepthSlider("Bit-Depth", "bit")
            , mDownsamplingSlider("Downsampling", "x")
            , mWetDrySlider("Mix", "")
            , mBitDepthAttachment(owner.state, "bitdepth", mBitDepthSlider)
            , mDownsamplingAttachment(owner.state, "downsampling", mDownsamplingSlider)
            , mWetDryAttachment(owner.state, "wetdry", mWetDrySlider)
            {
                addAndMakeVisible(&mBitDepthSlider);
                mBitDepthSlider.mLabels.add({0.0f, "3"});
                mBitDepthSlider.mLabels.add({1.0f, "24"});
                
                addAndMakeVisible(&mDownsamplingSlider);
                mDownsamplingSlider.mLabels.add({0.0f, "1"});
                mDownsamplingSlider.mLabels.add({1.0f, "10"});
                
                addAndMakeVisible(&mWetDrySlider);
                mWetDrySlider.mLabels.add({0.0f, "Dry"});
                mWetDrySlider.mLabels.add({1.0f, "Wet"});
                setSize (600, 200);
            }
            
            ~AudioDecayPluginProcessorEditor() override {}
            
            void paint(juce::Graphics& g) override
            {
                g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
            }
            
            void resized() override
            {
                auto bounds = getLocalBounds();
                bounds.reduced(20, 20);
                
                auto constexpr numUIElements = 3;
                auto const rotaryWidth = static_cast<int>(bounds.getWidth() * 1 / numUIElements);
                auto const spacing = (bounds.getWidth() - rotaryWidth * numUIElements) / (numUIElements - 1);
                
                mBitDepthSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
                bounds.removeFromLeft(spacing);
                mDownsamplingSlider.setBounds(bounds.removeFromLeft(rotaryWidth));
                bounds.removeFromLeft(spacing);
                mWetDrySlider.setBounds(bounds.removeFromLeft(rotaryWidth));
            }
            
        private:
            RotarySliderWithLabels mBitDepthSlider;
            RotarySliderWithLabels mDownsamplingSlider;
            RotarySliderWithLabels mWetDrySlider;
            
            juce::AudioProcessorValueTreeState::SliderAttachment mBitDepthAttachment;
            juce::AudioProcessorValueTreeState::SliderAttachment mDownsamplingAttachment;
            juce::AudioProcessorValueTreeState::SliderAttachment mWetDryAttachment;
        };
        
        //==============================================================================
        
        // juce::AudioProcessorValueTreeState::Listener
        void parameterChanged (const String& parameterID, float newValue) override;
        
        void updateQuantisationLevel(float bitDepth);
        
        //==============================================================================
        int mBlockSize;
        int mSampleRate;
        
        juce::AudioParameterFloat* mBitDepth;
        juce::AudioParameterInt* mDownsamplingFactor;
        juce::AudioParameterFloat* mWetDryMix;
        
        float mQuantisationLevel = 1.0;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDecayProcessor)
    };
}

