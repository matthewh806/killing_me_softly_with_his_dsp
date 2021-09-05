#include "SimpleDelayProcessor.h"

SimpleDelayProcessor::SimpleDelayProcessor()
: juce::AudioProcessor (BusesProperties().withInput  ("Input",     juce::AudioChannelSet::stereo())
                  .withOutput ("Output",    juce::AudioChannelSet::stereo())
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo()))
{
    addParameter (mWetDryMix = new AudioParameterFloat ("wetdrymix", "WetDryMix", 0.0f, 1.0f, 0.5f));
    addParameter (mDelayTime = new AudioParameterFloat ("delaytime", "DelayTime", 0.0f, 2.0f, 0.1f));
    addParameter (mFeedback = new AudioParameterFloat ("feedback", "Feedback", 0.0f, 1.0f, 0.5f));
}

//==============================================================================
bool SimpleDelayProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
             && ! layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void SimpleDelayProcessor::prepareToPlay (double sampleRate,
                                          int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    mDelayBuffers = new CircularBuffer<float> *[2];
    auto const bufferSize = static_cast<unsigned int>(2.0 * sampleRate) + 1; // avoid round down?
    mDelayBuffers[0] = new CircularBuffer<float>();
    mDelayBuffers[0]->createCircularBuffer(bufferSize);
    mDelayBuffers[1] = new CircularBuffer<float>();
    mDelayBuffers[1]->createCircularBuffer(bufferSize);
}

void SimpleDelayProcessor::releaseResources()
{
    
}

void SimpleDelayProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    auto const delayTime = mDelayTime->get();
    auto const wetDryRatio = mWetDryMix->get();
    auto const feedbackAmt = mFeedback->get();
    
    auto const channels = buffer.getNumChannels();
    if(channels > 2)
    {
        return;
    }
    
    auto const numSamples = buffer.getNumSamples();
    for(auto ch = 0; ch < channels; ++ch)
    {
        for(int sample = 0; sample < numSamples; ++sample)
        {
            auto const inputSignal = buffer.getSample(ch, sample);
            auto const delayedSample = static_cast<float>(mDelayBuffers[ch]->readBuffer(delayTime * mSampleRate)); // not thread safe...

            double inputToDelayBuffer = inputSignal + feedbackAmt * delayedSample;
            mDelayBuffers[ch]->writeBuffer(static_cast<float>(inputToDelayBuffer));
            buffer.setSample(ch, sample, (1.0f - wetDryRatio) * inputSignal + wetDryRatio * delayedSample);
        }
    }
}

void SimpleDelayProcessor::setWetDryMix(float newValue)
{
    *mWetDryMix = std::max(0.0f, std::min(newValue, 1.0f));
}

void SimpleDelayProcessor::setDelayTime(float newValue)
{
    *mDelayTime = std::max(0.0f, std::min(newValue, 2.0f));
}

void SimpleDelayProcessor::setFeedback(float newValue)
{
    *mFeedback = std::max(0.0f, std::min(newValue, 1.0f));
}

void SimpleDelayProcessor::getStateInformation (MemoryBlock& destData)
{
    MemoryOutputStream stream (destData, true);
}

void SimpleDelayProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);
}
