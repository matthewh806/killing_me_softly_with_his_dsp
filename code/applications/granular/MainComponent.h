#pragma once

#include "JuceHeader.h"
#include "../../ui/CustomLookAndFeel.h"
#include "../../ui/Waveform.h"
#include "../../core/ReferenceCountedBuffer.h"
#include "../../dsp/synthesis/granular/Scheduler.h"

class GranularWaveform
: public WaveformComponent
{
public:
    using WaveformComponent::WaveformComponent;
    
    // active, pos in sample, randomized y pixel pos, randomized colour
    using GrainInfo = std::array<std::tuple<bool, size_t, float, juce::Colour>, Scheduler::POOL_SIZE>;
    
    void paint(juce::Graphics& g) override
    {
        WaveformComponent::paint(g);
        
        // draw the grains
        auto const lengthInSeconds = getThumbnail().getTotalLength();
        auto const lengthInSamples = static_cast<size_t>(std::floor(lengthInSeconds * 44100.0));
        auto const waveformBounds = getLocalBounds();
        
        for(size_t i = 0; i < Scheduler::POOL_SIZE; ++i)
        {
            if(std::get<0>(mGrainInfo[i]) == false)
            {
                continue;
            }
            
            auto const samplePos = std::get<1>(mGrainInfo[i]);
            // convert from sample pos to screen pos
            
            auto const screenPos = static_cast<float>(samplePos) / lengthInSamples * static_cast<float>(waveformBounds.getWidth()) + static_cast<float>(waveformBounds.getX());
            g.setColour (std::get<3>(mGrainInfo[i]));
            g.drawEllipse (screenPos, std::get<2>(mGrainInfo[i]), 2, 2, 3);
        }
    }
    
    void updateGrainInfo(std::array<Grain, Scheduler::POOL_SIZE> const& grains)
    {
        auto& random = juce::Random::getSystemRandom();
        for(size_t i = 0; i < Scheduler::POOL_SIZE; ++i)
        {
            if(std::get<0>(mGrainInfo[i]) != !grains[i].isGrainComplete())
            {
                auto const waveformHeight = getThumbnailBounds().getHeight();
                auto const waveformY = getThumbnailBounds().getY();
                std::get<2>(mGrainInfo[i]) = static_cast<float>(random.nextInt(waveformHeight)) + static_cast<float>(waveformY);
                
                juce::Colour colour (random.nextInt (juce::Range<int> (100, 256)),
                                     random.nextInt (juce::Range<int> (50,  200)),
                                     200);
                std::get<3>(mGrainInfo[i]) = colour;
            }
            
            std::get<0>(mGrainInfo[i]) = !grains[i].isGrainComplete();
            std::get<1>(mGrainInfo[i]) = grains[i].getGrainPosition();
        }
        
        repaint();
    }
    
private:
    GrainInfo mGrainInfo;
};

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
    
    void updateEnvelopeEssence();
    
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
    
    GranularWaveform mWaveformComponent {mFormatManager};
    
    RotarySliderWithLabels mFrequencySlider;
    
    RotarySliderWithLabels mGrainAmplitudeSlider;
    RotarySliderWithLabels mEnvelopeAttackSlider;
    RotarySliderWithLabels mEnvelopeReleaseSlider;
    
    juce::SpinLock mMutex;
    juce::ReferenceCountedArray<ReferenceCountedBuffer> mBuffers;
    ReferenceCountedBuffer::Ptr mCurrentBuffer;
    
    Source::SourceType mSourceType {Source::SourceType::sample};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
