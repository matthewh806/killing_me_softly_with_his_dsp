#include "RulerTwangPlugin.h"

#define MAX_DELAY_SECONDS 2

using namespace OUS;

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RulerTwangPlugin();
}

RulerTwangPlugin::RulerTwangPlugin()
: juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                        .withOutput("Output", juce::AudioChannelSet::stereo()))
, mState(*this, nullptr, "pluginstate",
{
    std::make_unique<juce::AudioParameterBool>("triggertwang", "Trigger Twang", 0.0f),
    std::make_unique<juce::AudioParameterFloat>("decaytime", "Decay Time (ms)", 10.0f, 2000.0f, 450.0f),
    std::make_unique<juce::AudioParameterFloat>("youngsmodulus", "Youngs Modulus", 1000000000.0f, 200000000000.0f, 1000000000.0f),
    std::make_unique<juce::AudioParameterFloat>("rulerlength", "Ruler Length", 100.0f, 1000.0f, 300.0f),
    std::make_unique<juce::AudioParameterFloat>("rulerheight", "Ruler Height", 1.0f, 100.0f, 3.5f),
    std::make_unique<juce::AudioParameterFloat>("rulerdensity", "Ruler Density", 10.0f, 1000.0f, 750.0f),
    std::make_unique<juce::AudioParameterFloat>("freevibrationfrequency", "Free Vibration Frequency", 0.1f, 3000.0f, 220.0f),
    std::make_unique<juce::AudioParameterFloat>("clampedvibrationfrequency", "Clamped Vibration Frequency", 0.1f, 3000.0f, 220.0f),
})
, mFullClampedModes({1.0f, 6.2669f, 17.5475f, 34.3861f, 56.8426f})
{
    mState.addParameterListener("triggertwang", this);
    mState.addParameterListener("youngsmodulus", this);
    mState.addParameterListener("rulerlength", this);
    mState.addParameterListener("rulerheight", this);
    mState.addParameterListener("rulerdensity", this);
    mState.addParameterListener("freevibrationfrequency", this);
    mState.addParameterListener("clampedvibrationfrequency", this);
    mState.addParameterListener("decaytime", this);
    mState.state.addChild({"uiState", {{"width", 400}, {"height", 250}}, {}}, -1, nullptr);
    
    mSawtoothRamp.initialise([](float x) { return juce::jmap(x, -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f, 1.0f); }, 128);
    
    mLowpassFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    mLowpassFilter.setCutoffFrequency(4000);
    
    mHighpassFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    mHighpassFilter.setCutoffFrequency(90);
    
    mFullClampedModes.setLevel(0.2f);
}

RulerTwangPlugin::~RulerTwangPlugin()
{
    mState.removeParameterListener("decaytime", this);
    mState.removeParameterListener("freevibrationfrequency", this);
    mState.removeParameterListener("clampedvibrationfrequency", this);
    mState.removeParameterListener("youngsmodulus", this);
    mState.removeParameterListener("rulerlength", this);
    mState.removeParameterListener("rulerheight", this);
    mState.removeParameterListener("rulerdensity", this);
    mState.removeParameterListener("triggertwang", this);
}

//==============================================================================
bool RulerTwangPlugin::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet() && !layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void RulerTwangPlugin::prepareToPlay(double sampleRate,
                                         int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    auto const processSpec = juce::dsp::ProcessSpec {sampleRate, static_cast<uint32>(maximumExpectedSamplesPerBlock), 2 };
    
    auto const freeVibrationFrequency = calculateFundamentalFrequency(LAMBDA_FREE_FUNDAMENTAL);
    auto const clampedVibrationFrequency = calculateFundamentalFrequency(LAMBDA_CLAMPED_FUNDAMENTAL);
    
    auto* freeVibrationFreqParam = mState.getParameter("freevibrationfrequency");
    auto normalisedFreeFreq = freeVibrationFreqParam->convertTo0to1(freeVibrationFrequency);
    freeVibrationFreqParam->setValueNotifyingHost(normalisedFreeFreq);
    
    auto* clampedVibrationFreqParam = mState.getParameter("clampedvibrationfrequency");
    auto normalisedClampedFreq = freeVibrationFreqParam->convertTo0to1(clampedVibrationFrequency);
    clampedVibrationFreqParam->setValueNotifyingHost(normalisedClampedFreq);
    
    mSawtoothRamp.prepare(processSpec);
    mSawtoothRamp.setFrequency(clampedVibrationFrequency);
    
    mLowpassFilter.prepare(processSpec);
    mHighpassFilter.prepare(processSpec);
    
    mFullClampedModes.prepare(processSpec);
    
    mFreeVibrationModes.prepare(processSpec);
    mFreeVibrationModes.setFundamentalFrequency(freeVibrationFrequency);
    
    mSawtoothRampBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    mClampedBarBuffer.setSize(2, maximumExpectedSamplesPerBlock);
    mFreeBarBuffer.setSize(2, maximumExpectedSamplesPerBlock);
}

void RulerTwangPlugin::releaseResources()
{
}

void RulerTwangPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ignoreUnused(midiBuffer);
    
    buffer.clear();
    mSawtoothRampBuffer.clear();
    mClampedBarBuffer.clear();
    mFreeBarBuffer.clear();
    
    juce::dsp::AudioBlock<float> sawtoothRampBlock(mSawtoothRampBuffer, 0);
    mSawtoothRamp.process(juce::dsp::ProcessContextReplacing<float>(sawtoothRampBlock));
    
    juce::dsp::AudioBlock<float> clampedBlock(mClampedBarBuffer, 0);
    clampedBlock.copyFrom(sawtoothRampBlock);
    mFullClampedModes.process(juce::dsp::ProcessContextReplacing<float>(clampedBlock));
    
    auto const numSamples = buffer.getNumSamples();
    for(auto i = 0; i < numSamples; ++i)
    {
        // generate white noise
        auto const noiseVal = random.nextFloat() * 2.0f - 1.0f;
        auto const squaredRampVal = sawtoothRampBlock.getSample(0, i) * sawtoothRampBlock.getSample(0, i);
        auto const modulatedClampedVal = clampedBlock.getSample(0, i) * squaredRampVal;
        
        // modulate with clamped vibration values
        auto const freeBarExcitation = noiseVal * modulatedClampedVal;
        mFreeBarBuffer.setSample(0, i, freeBarExcitation);
        mFreeBarBuffer.setSample(1, i, freeBarExcitation);
    }
    
    juce::dsp::AudioBlock<float> freeBlock(mFreeBarBuffer, 0);
    juce::dsp::ProcessContextReplacing<float> freeBlockProcessingContext(freeBlock);
    // lpf mfreebarbuffer
    mLowpassFilter.process(freeBlockProcessingContext);
    // pass to free bar vibration processor
    mFreeVibrationModes.process(freeBlockProcessingContext);
    
    // sum the two modes and output!
    for(auto i = 0; i < numSamples; ++i)
    {
        auto const value = 0.8f * mFreeBarBuffer.getSample(0, i) + 0.2f * mClampedBarBuffer.getSample(0, i);
        buffer.setSample(0, i, value);
        buffer.setSample(1, i, value);
    }
    
    // hpf buffer
    juce::dsp::AudioBlock<float> outputBlock(buffer, 0);
    mHighpassFilter.process(juce::dsp::ProcessContextReplacing<float>(outputBlock));
}

void RulerTwangPlugin::getStateInformation(MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, true);
}

void RulerTwangPlugin::setStateInformation(const void* data, int sizeInBytes)
{
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
}

void RulerTwangPlugin::parameterChanged(const juce::String& parameterID, float newValue)
{
    if(parameterID == "youngsmodulus" || parameterID == "rulerlength" || parameterID == "rulerheight" || parameterID == "rulerdensity")
    {
        auto const freeFundamental = calculateFundamentalFrequency(LAMBDA_FREE_FUNDAMENTAL);
        auto const clampedFundamental = calculateFundamentalFrequency(LAMBDA_CLAMPED_FUNDAMENTAL);
        
        auto* freeVibrationFreqParam = mState.getParameter("freevibrationfrequency");
        auto normalisedFreeFreq = freeVibrationFreqParam->convertTo0to1(freeFundamental);
        freeVibrationFreqParam->setValueNotifyingHost(normalisedFreeFreq);
        
        auto* clampedVibrationFreqParam = mState.getParameter("clampedvibrationfrequency");
        auto normalisedClampedFreq = freeVibrationFreqParam->convertTo0to1(clampedFundamental);
        clampedVibrationFreqParam->setValueNotifyingHost(normalisedClampedFreq);
    }
    else if(parameterID == "freevibrationfrequency")
    {
        mFreeVibrationModes.setFundamentalFrequency(newValue);
        if(auto* ed = this->createEditorIfNeeded())
        {
            ed->repaint();
        }
    }
    else if(parameterID == "clampedvibrationfrequency")
    {
        mSawtoothRamp.setFrequency(newValue);
        mFreeVibrationModes.setFundamentalFrequency(newValue);
        if(auto* ed = this->createEditorIfNeeded())
        {
            ed->repaint();
        }
    }
    else if(parameterID == "triggertwang")
    {
        triggerSystem();
    }
    else if(parameterID == "decaytime")
    {
        mFullClampedModes.setDecayTime(newValue);
    }
}

float RulerTwangPlugin::calculateFundamentalFrequency(float lambda)
{
    auto constexpr oneOverSquareRootTwelve = 0.2886751346f;
    
    auto const youngsModulus = mState.getRawParameterValue("youngsmodulus")->load();
    auto const rulerHeight = mState.getRawParameterValue("rulerheight")->load() / 1000.0f;
    auto const rulerLength = mState.getRawParameterValue("rulerlength")->load() / 1000.0f;
    auto const rulerDensity = mState.getRawParameterValue("rulerdensity")->load() / 1000.0f;
    auto const kappa = oneOverSquareRootTwelve * rulerHeight;
    
    return std::sqrt(youngsModulus * kappa * kappa / rulerDensity) * lambda * lambda / (rulerLength * rulerLength * juce::MathConstants<float>::twoPi);
}

void RulerTwangPlugin::resetSystem()
{
    mSawtoothRamp.reset();
    mLowpassFilter.reset();
    mHighpassFilter.reset();
    mFullClampedModes.reset();
    mFreeVibrationModes.reset();
}

void RulerTwangPlugin::triggerSystem()
{
    // This class has an internal ADSR which essentially
    // retriggers the whole system
    resetSystem();
    mFullClampedModes.trigger();
}
