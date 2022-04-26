#include "RubberbandPitchShifter.h"

RubberbandPitchShifter::RubberbandPitchShifter(int sampleRate, size_t numChannels, int blockSize)
: mChannels(numChannels)
, mOutputBuffer (numChannels)
{
    RubberBand::RubberBandStretcher::Options ops = RubberBand::RubberBandStretcher::OptionProcessRealTime;
    mRubber = std::make_unique<RubberBand::RubberBandStretcher>(sampleRate, numChannels, ops);
    
    mInputBuffer.setSize(static_cast<int>(numChannels), blockSize);
    mInputBuffer.clear();
    mScratchBuffer.setSize(static_cast<int>(numChannels), blockSize);
    mScratchBuffer.clear();
    
    auto const bufferSize = blockSize + 8192 * 2;
    for(size_t ch = 0; ch < numChannels; ++ch)
    {
        mOutputBuffer[ch] = std::make_unique<RubberBand::RingBuffer<float>>(bufferSize);
        mOutputBuffer[ch]->reset();
    }
    
    // TODO: Check why this causes NAN issues, the original logic was copied from the LADSPA eg:
    // https://github.com/breakfastquay/rubberband/blob/default/ladspa-lv2/RubberBandPitchShifter.h
    //float const* readPtrs[2] = {mScratchBuffer.getReadPointer(0, 0), mScratchBuffer.getReadPointer(1, 0)};
//    mRubber->process(readPtrs, 8192, false);
}

RubberbandPitchShifter::~RubberbandPitchShifter()
{
}

void RubberbandPitchShifter::setPitchRatio(float ratio)
{
    mRubber->setPitchScale(ratio);
}

void RubberbandPitchShifter::process(AudioBuffer<float>& buffer, size_t numSamples)
{
    size_t processedSamples = 0;
    size_t outTotal = 0;
    
    for(int ch = 0; ch < static_cast<int>(mChannels); ++ch)
    {
        mInputBuffer.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));
    }

    while(processedSamples < numSamples)
    {
        auto const requiredSamples = mRubber->getSamplesRequired();
        auto const inChunk = std::min(numSamples - processedSamples, requiredSamples);
        
        std::vector<float const*> readPtrs(mChannels, nullptr);
        for(size_t ch = 0; ch < mChannels; ++ch)
        {
            readPtrs[ch] = mInputBuffer.getReadPointer(static_cast<int>(ch), static_cast<int>(processedSamples));
        }
        
        mRubber->process(readPtrs.data(), static_cast<size_t>(inChunk), false);
        processedSamples += inChunk;
        
        auto const availableSamples = mRubber->available();
        auto const writableSamples = mOutputBuffer[0]->getWriteSpace();
        
        if(availableSamples > writableSamples)
        {
//            std::cerr << "RubberbandPitchShifter::process: output buffer is not large enough. size = "
//            << mOutputBuffer[0]->getSize() << ", chunk = " << availableSamples << ", space = "
//            << writableSamples << " (buffer contains " << mOutputBuffer[0]->getReadSpace() << " unread)\n";
        }
        
        auto const outChunk = std::min(availableSamples, writableSamples);
        
        std::vector<float*> writePtrs(mChannels, nullptr);
        for(size_t ch = 0; ch < mChannels; ++ch)
        {
            writePtrs[ch] = mScratchBuffer.getWritePointer(static_cast<int>(ch), 0);
        }

        auto const retrieved = mRubber->retrieve(writePtrs.data(), static_cast<size_t>(outChunk));
        outTotal += retrieved;
        
        for(size_t ch = 0; ch < mChannels; ++ch)
        {
            mOutputBuffer[ch]->write(mScratchBuffer.getReadPointer(static_cast<int>(ch)), static_cast<int>(outTotal));
        }
    }
    
    auto const toRead = mOutputBuffer[0]->getReadSpace();
    if(toRead < static_cast<int>(numSamples))
    {
//        std::cerr << "RubberbandPitchShifter::process: buffer underrun: required = " << numSamples
//        << ", available = " << toRead << "\n";
    }
    
    auto const chunk = std::min(toRead, static_cast<int>(numSamples));
    
    for(size_t ch = 0; ch < mChannels; ++ch)
    {
        mOutputBuffer[ch]->read(buffer.getWritePointer(static_cast<int>(ch)), chunk);
    }
}
