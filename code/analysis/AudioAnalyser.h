#pragma once

#include <JuceHeader.h>
#include "aubio.h"

class AudioAnalyser
{
public:
    struct DetectionSettings
    {
        int windowSize = 1024;
        int hopSize = 256;
        int sampleRate = 44100;
    };
    
    std::vector<size_t> static getOnsetPositions(juce::AudioBuffer<float> buffer, DetectionSettings settings);
    
private:
};
