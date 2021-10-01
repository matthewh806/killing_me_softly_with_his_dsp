#include "AudioAnalyser.h"

std::vector<size_t> AudioAnalyser::getOnsetPositions(juce::AudioBuffer<float> buffer, DetectionSettings settings)
{
    juce::ignoreUnused(buffer, settings);
    
    fvec_t* vecIn = new_fvec(static_cast<int>(settings.hopSize));
    fvec_t* vecOut = new_fvec(2);
    aubio_onset_t* onset = new_aubio_onset("default", settings.windowSize, settings.hopSize, settings.sampleRate);
    
    auto const numSamples = buffer.getNumSamples();
    auto samplesRemaining = numSamples;
    auto currentPosition = 0;
    
    std::vector<size_t> onsetPositions = {};
    while(samplesRemaining > 0)
    {
        auto const numThisTime = std::min(samplesRemaining, settings.hopSize);
        for(int i = 0; i < numThisTime; ++i)
        {
            vecIn->data[i] = buffer.getSample(0, currentPosition + i);
        }
        
        for(int i = numThisTime; i < settings.hopSize; ++i)
        {
            vecIn->data[i] = 0.0;
        }
        
        aubio_onset_do(onset, vecIn, vecOut);
        if(vecOut->data[0] != 0)
        {
            // got some onset data!
            onsetPositions.push_back(static_cast<size_t>(aubio_onset_get_last(onset)));
        }
        
        currentPosition += numThisTime;
        samplesRemaining -= numThisTime;
    }
    
    del_aubio_onset(onset);
    del_fvec(vecIn);
    del_fvec(vecOut);
    
    return onsetPositions;
}
