#pragma once

#include "aubio.h"
#include <JuceHeader.h>

namespace OUS
{
    class AudioAnalyser
    {
    public:
        struct DetectionSettings
        {
            int windowSize = 1024;
            int hopSize = 256;
            int sampleRate = 44100;
            smpl_t threshold = 0.3f;
        };

        std::vector<size_t> static getOnsetPositions(juce::AudioBuffer<float> buffer, DetectionSettings settings);

    private:
    };
} // namespace OUS
