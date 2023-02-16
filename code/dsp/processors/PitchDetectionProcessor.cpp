#include "PitchDetectionProcessor.h"

#define MIN_DETECTABLE_FREQUENCY 27.50f // This is the minimum frequency we can reliably find (affects latency!)

using namespace OUS;

static float getMinBlockSizeToDetectFrequency(float frequency, float sampleRate)
{
    return 2.0f * sampleRate / frequency;
}

PitchDetectionProcessor::PitchDetectionProcessor()
: juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo()))
, mState(*this, nullptr, "pitchdetectionprocessorstate",
         {std::make_unique<juce::AudioParameterFloat>("detectedpitch", "Detected Pitch", 0.0f, 22050.0f, 0.0f)})
{
    mOutputVector = new_fvec(1);
    assert(mOutputVector != nullptr);
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

    if(mInputSamples != nullptr)
    {
        del_fvec(mInputSamples);
        mInputSamples = nullptr;
    }

    auto const pitchProcessorBlockSize = static_cast<int>(std::ceil(getMinBlockSizeToDetectFrequency(MIN_DETECTABLE_FREQUENCY, static_cast<float>(sampleRate))));
    setLatencySamples(pitchProcessorBlockSize);
    mInputSamples = new_fvec(static_cast<uint_t>(pitchProcessorBlockSize));
    mCircularBuffer.createCircularBuffer(static_cast<uint_t>(pitchProcessorBlockSize));

    mAudioPitch = new_aubio_pitch("yin", static_cast<uint_t>(pitchProcessorBlockSize), static_cast<uint_t>(pitchProcessorBlockSize), static_cast<uint_t>(sampleRate));
    assert(mAudioPitch != nullptr);
}

void PitchDetectionProcessor::releaseResources()
{
    if(mInputSamples != nullptr)
    {
        del_fvec(mInputSamples);
        mInputSamples = nullptr;
    }

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

    // Copy new samples into buffer
    for(int i = 0; i < numSamples; ++i)
    {
        mCircularBuffer.writeBuffer(buffer.getSample(0, i));
    }

    auto const latency = getLatencySamples();
    for(int i = 0; i < static_cast<int>(mInputSamples->length); ++i)
    {
        fvec_set_sample(mInputSamples, mCircularBuffer.readBuffer(latency - i), static_cast<uint_t>(i));
    }

    aubio_pitch_do(mAudioPitch, mInputSamples, mOutputVector);

    auto lastDetectedPitchPtr = static_cast<juce::AudioParameterFloat*>(mState.getParameter("detectedpitch"));
    *lastDetectedPitchPtr = *mOutputVector->data;
}

void PitchDetectionProcessor::getStateInformation(MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, true);
}

void PitchDetectionProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
}
