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
    std::make_unique<juce::AudioParameterFloat>("decaytime", "Decay Time (ms)", 10.0f, 2000.0f, 450.0f),
    std::make_unique<juce::AudioParameterFloat>("youngsmodulus", "Youngs Modulus", 1000000000.0f, 200000000000.0f, 1000000000.0f),
    std::make_unique<juce::AudioParameterFloat>("rulerlength", "Ruler Length", 100.0f, 1000.0f, 300.0f),
    std::make_unique<juce::AudioParameterFloat>("rulerheight", "Ruler Height", 1.0f, 100.0f, 3.5f),
    std::make_unique<juce::AudioParameterFloat>("rulerdensity", "Ruler Density", 10.0f, 1000.0f, 750.0f),
    std::make_unique<juce::AudioParameterFloat>("vibrationmodebalance", "Vibration Mode", 0.0f, 1.0f, 0.5f),
    std::make_unique<juce::AudioParameterBool>("modulatefreevibrations", "Modulate Free vibrations", 1.0f)
})
{
    mState.addParameterListener("triggertwang", this);
    mState.addParameterListener("youngsmodulus", this);
    mState.addParameterListener("rulerlength", this);
    mState.addParameterListener("rulerheight", this);
    mState.addParameterListener("rulerdensity", this);
    mState.addParameterListener("freevibrationfrequency", this);
    mState.addParameterListener("clampedvibrationfrequency", this);
    mState.addParameterListener("decaytime", this);
    mState.addParameterListener("vibrationmodebalance", this);
    mState.addParameterListener("modulatefreevibrations", this);
    mState.state.addChild({"uiState", {{"width", 400}, {"height", 250}}, {}}, -1, nullptr);
}

RulerTwangPlugin::~RulerTwangPlugin()
{
    mState.removeParameterListener("modulatefreevibrations", this);
    mState.removeParameterListener("vibrationmodebalance", this);
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
    
    mRulerVibrationalModes.prepare(processSpec);
}

void RulerTwangPlugin::releaseResources()
{
}

void RulerTwangPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
    juce::ignoreUnused(midiBuffer);
    
    juce::dsp::AudioBlock<float> outputBlock(buffer, 0);
    mRulerVibrationalModes.process(juce::dsp::ProcessContextReplacing<float>(outputBlock));
}

void RulerTwangPlugin::getStateInformation(MemoryBlock& destData)
{
    auto state = mState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RulerTwangPlugin::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if(xmlState.get() != nullptr)
    {
        if(xmlState->hasTagName(mState.state.getType()))
        {
            mState.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

void RulerTwangPlugin::parameterChanged(const juce::String& parameterID, float newValue)
{
    if(parameterID == "youngsmodulus" || parameterID == "rulerlength" || parameterID == "rulerheight" || parameterID == "rulerdensity")
    {
        auto const youngsModulus = mState.getRawParameterValue("youngsmodulus")->load();
        auto const height = mState.getRawParameterValue("rulerheight")->load() / 1000.0f;
        auto const length = mState.getRawParameterValue("rulerlength")->load() / 1000.0f;
        auto const density = mState.getRawParameterValue("rulerdensity")->load() / 1000.0f;
        
        mRulerVibrationalModes.setFundamentalFrequency(youngsModulus, height, length, density);
        
        if(auto* editor = getActiveEditor())
        {
            dynamic_cast<RulerTwangPluginProcessorEditor*>(editor)->setFreeFrequency(mRulerVibrationalModes.getFreeFundamentalFrequency());
        }
        
        if(auto* editor = getActiveEditor())
        {
            dynamic_cast<RulerTwangPluginProcessorEditor*>(editor)->setClampedFrequency(mRulerVibrationalModes.getClampedFundamentalFrequency());
        }
    }
    else if(parameterID == "decaytime")
    {
        mRulerVibrationalModes.setDecayTime(newValue);
    }
    else if(parameterID == "vibrationmodebalance")
    {
        mRulerVibrationalModes.setVibrationalModeBalance(newValue);
    }
    else if(parameterID == "modulatefreevibrations")
    {
        mRulerVibrationalModes.setModulateFreeVibrationalModes(newValue == 0.0 ? false : true);
    }
}

void RulerTwangPlugin::resetSystem()
{
    mRulerVibrationalModes.reset();
}

void RulerTwangPlugin::triggerSystem()
{
    mRulerVibrationalModes.triggerSystem();
}

juce::File RulerTwangPlugin::getPresetsFolder()
{
    File rootFolder = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory);
    
#ifdef JUCE_MAC
    rootFolder = rootFolder.getChildFile("Audio").getChildFile("Presets");
#endif
    
    rootFolder = rootFolder.getChildFile("the office of unspecified services").getChildFile("RulerTwang");
    Result res = rootFolder.createDirectory();
    
    return rootFolder;
}

void RulerTwangPlugin::savePreset()
{
    mFileChooser = std::make_unique<juce::FileChooser>("Save Preset",
                                                       getPresetsFolder(), "*.xml");
    
    auto folderChooserFlags = FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles;
    mFileChooser->launchAsync(folderChooserFlags, [this](juce::FileChooser const& chooser)
    {
        auto file(chooser.getResult());
        try
        {
            auto state = mState.copyState();
            std::unique_ptr<juce::XmlElement> xml(state.createXml());
            xml->writeTo(file);
        }
        catch (std::exception e)
        {
            juce::AlertWindow::showAsync(MessageBoxOptions()
                                             .withIconType(MessageBoxIconType::WarningIcon)
                                             .withTitle("Failed to save preset")
                                             .withMessage(e.what())
                                             .withButton("OK"),
                                         nullptr);
        }
    });
}

void RulerTwangPlugin::loadPreset(juce::String presetName)
{
    File presetToLoad = getPresetsFolder().getChildFile(presetName + ".xml");
    if(presetToLoad.existsAsFile())
    {
        std::unique_ptr presetXml = juce::XmlDocument::parse(presetToLoad);
        if(presetXml.get() != nullptr)
        {
            if(presetXml->hasTagName(mState.state.getType()))
            {
                mState.replaceState(juce::ValueTree::fromXml(*presetXml));
            }
        }
    }
}
