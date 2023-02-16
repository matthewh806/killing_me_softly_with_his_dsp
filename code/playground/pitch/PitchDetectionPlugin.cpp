#include "PitchDetectionPlugin.h"

using namespace OUS;

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchDetectionPlugin();
}

PitchDetectionPlugin::PitchDetectionPlugin()
: juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo()).withOutput("Output", juce::AudioChannelSet::stereo()))
, mState(*this, nullptr, "pluginstate", {})
{
    mState.state.addChild({"uiState", {{"width", 400}, {"height", 250}}, {}}, -1, nullptr);
    startTimerHz(30);
}

//==============================================================================
bool PitchDetectionPlugin::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() && !layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void PitchDetectionPlugin::prepareToPlay(double sampleRate,
                                         int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);

    mPitchDetectionProcessor.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void PitchDetectionPlugin::releaseResources()
{
    mPitchDetectionProcessor.releaseResources();
}

void PitchDetectionPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    // TODO: Check bypass state
    mPitchDetectionProcessor.processBlock(buffer, midiBuffer);
}

void PitchDetectionPlugin::getStateInformation(MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, true);
}

void PitchDetectionPlugin::setStateInformation(const void* data, int sizeInBytes)
{
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
}

void PitchDetectionPlugin::timerCallback()
{
    if(auto* editor = dynamic_cast<PitchDetectionPluginEditor*>(getActiveEditor()))
    {
        auto const pitch = mPitchDetectionProcessor.getMostRecentPitch();
        editor->setPitch(pitch);
    }
}
