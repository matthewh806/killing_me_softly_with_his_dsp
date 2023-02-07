#pragma once

#include "../../core/RingBuffer.h"
#include "../../dependencies/rubberband/rubberband/RubberBandStretcher.h"
#include "../JuceLibraryCode/JuceHeader.h"

namespace OUS
{
    class RubberbandPitchShifter
    {
    public:
        RubberbandPitchShifter(int sampleRate, size_t numChannels, int blockSize);
        ~RubberbandPitchShifter();

        size_t getLatency();

        void setPitchRatio(float ratio);
        void process(AudioBuffer<float>& buffer, size_t numSamples);

    private:
        size_t mChannels;

        juce::AudioBuffer<float> mInputBuffer;
        juce::AudioBuffer<float> mScratchBuffer;
        std::vector<std::unique_ptr<RubberBand::RingBuffer<float>>> mOutputBuffer;

        std::unique_ptr<RubberBand::RubberBandStretcher> mRubber;
    };
} // namespace OUS
