#include "PitchDetectionProcessor.h"

#define HOP_SIZE 256

using namespace OUS;

PitchDetectionProcessor::PitchDetectionProcessor()
: juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo()))
, mState(*this, nullptr, "pitchdetectionprocessorstate",
         {std::make_unique<juce::AudioParameterFloat>("detectedpitch", "Detected Pitch", 0.0f, 22050.0f, 0.0f)})
{
}

float PitchDetectionProcessor::getMostRecentPitch() const
{
    return static_cast<juce::AudioParameterFloat*>(mState.getParameter("detectedpitch"))->get();
}

//==============================================================================
bool PitchDetectionProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() && !layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void PitchDetectionProcessor::prepareToPlay(double sampleRate,
                                         int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    jassert(HOP_SIZE <= maximumExpectedSamplesPerBlock);

    mAudioPitch = new_aubio_pitch("yin", static_cast<uint_t>(maximumExpectedSamplesPerBlock), HOP_SIZE, static_cast<uint_t>(sampleRate));
    assert(mAudioPitch != nullptr);
}

void PitchDetectionProcessor::releaseResources()
{
    if(mAudioPitch != nullptr)
    {
        del_aubio_pitch(mAudioPitch);
        mAudioPitch = nullptr;
    }
}

void PitchDetectionProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // TODO: Stereo ?
    
    auto const numSamples = buffer.getNumSamples();
    auto samplesRemaining = numSamples;
    
    // store these as members for efficiency
    auto inputVector = new_fvec(HOP_SIZE);
    auto outputVector = new_fvec(1);
    
    auto lastDetectedPitchPtr = static_cast<juce::AudioParameterFloat*>(mState.getParameter("detectedpitch"));
    auto sampleOffset = 0;
    while(samplesRemaining > 0)
    {
        // TODO: What if we have less than HOP_SIZE? Does it still work?
        auto const numThisTime = std::max(HOP_SIZE, samplesRemaining);
        
        assert(sampleOffset + numThisTime <= numSamples);
        // copy samples into input Vector (or just point to them?)
        for(int i = 0; i  < numThisTime; ++i)
        {
            fvec_set_sample(inputVector, buffer.getSample(0, sampleOffset + i), static_cast<uint_t>(i));
        }
        aubio_pitch_do(mAudioPitch, inputVector, outputVector);
        *lastDetectedPitchPtr = *outputVector->data;
        
        samplesRemaining -= numThisTime;
        sampleOffset += numThisTime;
    }
}

void PitchDetectionProcessor::getStateInformation(MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, true);
}

void PitchDetectionProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
}

