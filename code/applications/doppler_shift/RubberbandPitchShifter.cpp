#include "RubberbandPitchShifter.h"

RubberbandPitchShifter::RubberbandPitchShifter(int sampleRate, size_t numChannels, int blockSize)
{
    RubberBand::RubberBandStretcher::Options ops = RubberBand::RubberBandStretcher::OptionProcessRealTime;
    mRubber = std::make_unique<RubberBand::RubberBandStretcher>(sampleRate, numChannels, ops);
    
    mInputBuffer.setSize(numChannels, blockSize);
    mScratchBuffer.setSize(numChannels, blockSize);
    mScratchBuffer.clear();
    
    mOutputBuffer = new RubberBand::RingBuffer<float> *[2];
    auto bufferSize = blockSize + 8192 * 2;
    mOutputBuffer[0] = new RubberBand::RingBuffer<float>(bufferSize);
    mOutputBuffer[1] = new RubberBand::RingBuffer<float>(bufferSize);
    
    float const* readPtrs[2] = {mScratchBuffer.getReadPointer(0, 0), mScratchBuffer.getReadPointer(1, 0)};
    mRubber->process(readPtrs, 8192, false);
}

void RubberbandPitchShifter::process(AudioBuffer<float>& buffer, size_t numSamples)
{
    size_t processedSamples = 0;
    size_t outTotal = 0;
    
    mInputBuffer.copyFrom(0, 0, buffer, 0, 0, static_cast<int>(numSamples));
    mInputBuffer.copyFrom(1, 0, buffer, 1, 0, static_cast<int>(numSamples));

    while(processedSamples < numSamples)
    {
        auto const requiredSamples = mRubber->getSamplesRequired();
        auto const inChunk = std::min(numSamples - processedSamples, requiredSamples);
        
        float const* readPtrs[2] = {mInputBuffer.getReadPointer(0, processedSamples), mInputBuffer.getReadPointer(1, processedSamples)};
        mRubber->process(readPtrs, static_cast<size_t>(inChunk), false);
        processedSamples += inChunk;
        
        auto const availableSamples = mRubber->available();
        auto const writableSamples = mOutputBuffer[0]->getWriteSpace();
        
        if(availableSamples > writableSamples)
        {
            std::cerr << "RubberbandPitchShifter::process: output buffer is not large enough. size = "
            << mOutputBuffer[0]->getSize() << ", chunk = " << availableSamples << ", space = "
            << writableSamples << " (buffer contains " << mOutputBuffer[0]->getReadSpace() << " unread)\n";
        }
        
        auto const outChunk = std::min(availableSamples, writableSamples);
        
        float* writePtrs[2] = {mScratchBuffer.getWritePointer(0, 0), mScratchBuffer.getWritePointer(1, 0)};
        auto const retrieved = mRubber->retrieve(writePtrs, static_cast<size_t>(outChunk));
        outTotal += retrieved;
        mOutputBuffer[0]->write(mScratchBuffer.getReadPointer(0), static_cast<int>(outTotal));
        mOutputBuffer[1]->write(mScratchBuffer.getReadPointer(1), static_cast<int>(outTotal));
    }
    
    auto const toRead = mOutputBuffer[0]->getReadSpace();
    if(toRead < numSamples)
    {
        std::cerr << "RubberbandPitchShifter::process: buffer underrun: required = " << numSamples
        << ", available = " << toRead << "\n";
    }
    
    auto const chunk = std::min(toRead, static_cast<int>(numSamples));
    mOutputBuffer[0]->read(buffer.getWritePointer(0), chunk);
    mOutputBuffer[1]->read(buffer.getWritePointer(1), chunk);
}
