#include "RealTimeStretchProcessor.h"

RealTimeStretchProcessor::RealTimeStretchProcessor()
: juce::AudioProcessor (BusesProperties().withInput  ("Input",     juce::AudioChannelSet::stereo())
                  .withOutput ("Output",    juce::AudioChannelSet::stereo())
                  .withInput  ("Sidechain", juce::AudioChannelSet::stereo()))
{
    addParameter (mStretchFactor = new AudioParameterFloat ("stretchfactor", "Stretch Factor", 0.1f, 4.0f, 0.1f));
    addParameter (mPitchShift = new AudioParameterFloat ("pitchshift", "Pitch Shift", 0.0f, 4.0f, 0.1f));
}

//==============================================================================
bool RealTimeStretchProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
             && ! layouts.getMainInputChannelSet().isDisabled();
}

//==============================================================================
void RealTimeStretchProcessor::prepareToPlay (double sampleRate,
                                          int maximumExpectedSamplesPerBlock)
{
    mBlockSize = maximumExpectedSamplesPerBlock;
    mSampleRate = static_cast<int>(sampleRate);
    
    mInput.setSize(2, maximumExpectedSamplesPerBlock);
    
    mOutputBuffer = new RubberBand::RingBuffer<float> *[2];
    auto bufferSize = maximumExpectedSamplesPerBlock + 1024 + 8192;
    mOutputBuffer[0] = new RubberBand::RingBuffer<float>(bufferSize);
    mOutputBuffer[1] = new RubberBand::RingBuffer<float>(bufferSize);
    
    mScratchBuffer.setSize(2, bufferSize);
    
    RubberBand::RubberBandStretcher::Options ops = RubberBand::RubberBandStretcher::OptionProcessRealTime | RubberBand::RubberBandStretcher::OptionPitchHighConsistency;
    std::unique_ptr<RubberBand::RubberBandStretcher> newBand (new RubberBand::RubberBandStretcher(static_cast<size_t>(sampleRate), static_cast<size_t>(2), ops));
    mRubberBand.reset(newBand.release());
    mRubberBand->setTimeRatio(1.0);
}

void RealTimeStretchProcessor::releaseResources()
{
    
}

void RealTimeStretchProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    auto const samples = buffer.getNumSamples();
    auto processedSamples = 0;
    
    mInput.clear();

    while(processedSamples < samples)
    {
        // Provide new samples and process
        auto required = mRubberBand->getSamplesRequired();
        auto inChunk = std::min(samples - processedSamples, static_cast<int>(required));
        
        const float* readPtrs[2] = {mInput.getReadPointer(0, processedSamples), mInput.getReadPointer(1, processedSamples)};
        mRubberBand->process(readPtrs, static_cast<size_t>(inChunk), false);
        processedSamples += inChunk;
        
        // write into output buffer
        auto availableSamples = mRubberBand->available();
        auto writableSamples = mOutputBuffer[0]->getWriteSpace();
        
        if(writableSamples == 0)
        {
            std::cout << "OutputBuffer is full! - no space left to write" << "\n";
        }
        
        auto outChunk = std::min(availableSamples, writableSamples);
        
        float* writePtrs[2] = {mScratchBuffer.getWritePointer(0, 0), mScratchBuffer.getWritePointer(1, 0)};
        auto retrieved = mRubberBand->retrieve(writePtrs, static_cast<size_t>(outChunk));
        
        std::cout << "available: " << availableSamples << ", outChunk: " << outChunk;
        if(retrieved != outChunk)
        {
            std::cout << " (" << retrieved << ")";
        }
        std::cout << "\n";
        
        outChunk = static_cast<int>(retrieved);
        
        for(size_t c = 0; c < 2; ++c)
        {
            if(mOutputBuffer    [c]->getWriteSpace() < outChunk)
            {
                std::cerr << "RubberBandPitchShifter::runImpl: buffer overrun: chunk = "
                    << outChunk << ", space = " << mOutputBuffer[c]->getWriteSpace() << std::endl;
            }
            mOutputBuffer[c]->write(mScratchBuffer.getReadPointer(static_cast<int>(c)), outChunk);
        }
    }
    
    // Finally read back the data from the output buffer
    for(size_t c = 0; c < 2; ++c)
    {
        auto const toRead = mOutputBuffer[c]->getReadSpace();
        if(toRead < samples && c == 0)
        {
            std::cerr << "RubberBandPitchShifter::runImpl: buffer underrun: required = " << samples << ", available = " << toRead << std::endl;
        }
        
        auto chunk = std::min(toRead, samples);
        mOutputBuffer[c]->read(buffer.getWritePointer(c), chunk);
    }
    
//    if (mMinfill == 0)
//    {
//        mMinfill = mOutputBuffer[0]->getReadSpace();
//    }
}

void RealTimeStretchProcessor::setStretchFactor(float newValue)
{
    *mStretchFactor = std::max(0.1f, std::min(newValue, 4.0f));
    mRubberBand->setTimeRatio(newValue);
}

void RealTimeStretchProcessor::setPitchShift(float newValue)
{
    *mPitchShift = std::max(0.0f, std::min(newValue, 4.0f));
    mRubberBand->setPitchScale(newValue);
}

void RealTimeStretchProcessor::getStateInformation (MemoryBlock& destData)
{
    MemoryOutputStream stream (destData, true);
}

void RealTimeStretchProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);
}
