#pragma once

// clang-format off
#include "JuceHeader.h"
// clang-format on

#include "../../core/ReferenceCountedBuffer.h"
#include "../../dsp/synthesis/granular/Scheduler.h"
#include "../../ui/CustomLookAndFeel.h"
#include "../../ui/WaveformAndRuler.h"

namespace OUS
{
    class GranularWaveform
    : public WaveformAndRuler
    {
    public:
        
        GranularWaveform(juce::AudioFormatManager& formatManager);

        // active, pos in sample, randomized y pixel pos, randomized colour
        using GrainInfo = std::array<std::tuple<bool, size_t, float, juce::Colour>, Scheduler::POOL_SIZE>;

        void paint(juce::Graphics& g) override;

        void updateGrainInfo(std::array<Grain, Scheduler::POOL_SIZE> const& grains);

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

        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override;
        void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

        //==============================================================================
        void paint(juce::Graphics&) override;
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

        GranularWaveform mWaveformComponent{mFormatManager};

        RotarySliderWithLabels mFrequencySlider;

        RotarySliderWithLabels mGrainAmplitudeSlider;
        RotarySliderWithLabels mEnvelopeAttackSlider;
        RotarySliderWithLabels mEnvelopeReleaseSlider;

        juce::SpinLock mMutex;
        juce::ReferenceCountedArray<ReferenceCountedBuffer> mBuffers;
        ReferenceCountedBuffer::Ptr mCurrentBuffer;

        Source::SourceType mSourceType{Source::SourceType::sample};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
    };
} // namespace OUS
