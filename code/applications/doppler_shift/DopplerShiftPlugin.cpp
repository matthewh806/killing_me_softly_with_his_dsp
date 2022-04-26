#include "DopplerShiftPlugin.h"

DopplerShiftProcessor::DopplerShiftProcessor()
: AudioProcessor (BusesProperties().withInput  ("Input",  AudioChannelSet::stereo())
                                    .withOutput ("Output", AudioChannelSet::stereo()))
{
    addParameter (mSourceSpeed = new AudioParameterFloat ("sourceSpeed", "SourceSpeed", 0.0f, 344.0f, 10.0f));
    startTimerHz(30);
}

    //==============================================================================
void DopplerShiftProcessor::prepareToPlay (double sampleRate, int samplesPerBlockExpected)
{
    mPitchShifter = std::make_unique<RubberbandPitchShifter>(sampleRate, 2, samplesPerBlockExpected);
}

void DopplerShiftProcessor::releaseResources()
{
}

void DopplerShiftProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiBuffer)
{
    mPitchShifter->setPitchRatio(mFrequencyRatio);
    mPitchShifter->process(buffer, buffer.getNumSamples());
}

//==============================================================================
AudioProcessorEditor* DopplerShiftProcessor::createEditor()
{
    auto* editor = new DopplerShiftPluginEditor(*this);
    // TODO: Not sure this is such a good idea - but is needed to initialise positions correctly
    editor->setInitialPositions(mSourcePosition, mObserverPosition);
    return editor;
}
bool DopplerShiftProcessor::hasEditor() const
{
    return true;
}

//==============================================================================
const String DopplerShiftProcessor::getName() const
{
    return "DopplerShift PlugIn";
}

bool DopplerShiftProcessor::acceptsMidi() const
{
    return false;
}

bool DopplerShiftProcessor::producesMidi() const
{
    return false;
}

double DopplerShiftProcessor::getTailLengthSeconds() const
{
    return 0;
}

//==============================================================================
int DopplerShiftProcessor::getNumPrograms()
{
    return 1;
}

int DopplerShiftProcessor::getCurrentProgram()
{
    return 0;
}

void DopplerShiftProcessor::setCurrentProgram (int)
{
    
}

const String DopplerShiftProcessor::getProgramName (int)
{
    return {};
}

void DopplerShiftProcessor::changeProgramName (int, const String&)
{
    
}

//==============================================================================
void DopplerShiftProcessor::getStateInformation (MemoryBlock& destData)
{
    //MemoryOutputStream (destData, true).writeFloat (*mSourceFrequency);
}

void DopplerShiftProcessor::setStateInformation (const void* data, int sizeInBytes)
{
   // mSourceFrequency->setValueNotifyingHost (MemoryInputStream (data, static_cast<size_t> (sizeInBytes), false).readFloat());
}

//==============================================================================
bool DopplerShiftProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainInLayout  = layouts.getChannelSet (true,  0);
    const auto& mainOutLayout = layouts.getChannelSet (false, 0);

    return (mainInLayout == mainOutLayout && (! mainInLayout.isDisabled()));
}

void DopplerShiftProcessor::timerCallback()
{
    auto* editor = dynamic_cast<DopplerShiftPluginEditor*>(getActiveEditor());
    auto const drawingViewHalfWidth = editor->getDrawingViewWidth() / 2.0f;
    
    auto const timerInterval = getTimerInterval();
    float constexpr speedOfSound = 344.0f / 1000.0f;
    
    const auto srcSpeed = mSourceSpeed->get() / 1000.0f;
    const auto srcVelocity = srcSpeed * mSourceDirection;
    
    auto const prevSourceXPosition = mSourcePosition.getX();
    auto const newSourceXPositon = mSourcePosition.getX() + timerInterval * srcVelocity;
    if(newSourceXPositon >= drawingViewHalfWidth || newSourceXPositon <= -drawingViewHalfWidth)
    {
        mSourceDirection *= -1;
    }
    
    mSourcePosition = { std::clamp(newSourceXPositon, -drawingViewHalfWidth, drawingViewHalfWidth), mSourcePosition.getY()};
    
    // work out angle based on source distance
    float angleRelativeToObserver = std::atan2(mObserverPosition.getY(), mSourcePosition.getX());
    float radialSpeed = srcVelocity * std::cos(angleRelativeToObserver);
    mFrequencyRatio = speedOfSound / (speedOfSound - radialSpeed);
    
    editor->updatePositions({mSourcePosition.getX() - prevSourceXPosition, 0.0f});
    
    std::cout
    << "velocity=" << srcVelocity
    << ", distance=" << mSourcePosition.getX()
    << ", angle=" << juce::radiansToDegrees(angleRelativeToObserver)
    << ", frequencyRatio=" << mFrequencyRatio
    << "\n";
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DopplerShiftProcessor();
}
