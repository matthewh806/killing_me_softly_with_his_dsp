#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../../core/RingBuffer.h"
#include "../../dependencies/rubberband/rubberband/RubberBandStretcher.h"

class RubberbandPitchShifter
{
public:
    RubberbandPitchShifter(int sampleRate, size_t numChannels, int blockSize);
    
    void process(AudioBuffer<float>& buffer, size_t numSamples);
    
private:
    juce::AudioBuffer<float> mInputBuffer;
    juce::AudioBuffer<float> mScratchBuffer;
    RubberBand::RingBuffer<float>** mOutputBuffer;
    
    std::unique_ptr<RubberBand::RubberBandStretcher> mRubber;
};
