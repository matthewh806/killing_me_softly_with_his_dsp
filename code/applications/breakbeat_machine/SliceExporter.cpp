/*
  ==============================================================================

    SliceExporter.cpp
    Created: 20 Jul 2020 12:35:28am
    Author:  Matthew

  ==============================================================================
*/

#include "SliceExporter.h"

using namespace OUS;

SliceExporter::SliceExporter(juce::AudioFormatManager& audioFormatManager)
: mFormatManager(audioFormatManager)
{
}

void SliceExporter::startExport(juce::AudioSampleBuffer* readBuffer, juce::String& fileName, juce::String& absoluteDirectoryPath, std::vector<SliceManager::Slice> const& slices, int numChannels, double sampleRate, int bitDepth, const int blockSize)
{
    auto const numSlices = slices.size();
    
    jassert(numSlices > 0 && readBuffer != nullptr);
    if(numSlices <= 0)
    {
        throw std::runtime_error("The number of slices is <= 0");
    }

    if(readBuffer == nullptr)
    {
        throw std::runtime_error("Invalid read buffer");
    }

    auto* audioFormat = mFormatManager.findFormatForFileExtension("wav");
    if(audioFormat == nullptr)
    {
        throw std::runtime_error("The audio format is not supported");
    }

    mReadBuffer.setSize(readBuffer->getNumChannels(), static_cast<int>(readBuffer->getNumSamples()));
    mReadBuffer.clear();
    for(int ch = 0; ch < readBuffer->getNumChannels(); ++ch)
    {
        mReadBuffer.copyFrom(ch, 0, *readBuffer, ch, 0, readBuffer->getNumSamples());
    }

    std::cout << "SliceExpoter::startExport: " << fileName << ", " << absoluteDirectoryPath << ", slice num / size: " << numSlices << "\n";

    for(size_t i = 0; i < numSlices; i++)
    {
        auto filePath = absoluteDirectoryPath + "/" + fileName + "_" + (i < 10 ? "0" : "") + juce::String(i) + ".wav";
        juce::File f{filePath};

        auto sliceStart = static_cast<int>(std::get<1>(slices[i]));
        auto sliceEnd = std::min(static_cast<int>(std::get<2>(slices[i])), readBuffer->getNumSamples());

        jassert(sliceStart >= 0 && sliceEnd >= sliceStart);

        if(f.exists() && !f.deleteFile())
        {
            throw std::runtime_error("The file is invalid");
        }

        auto outputStream = std::unique_ptr<juce::FileOutputStream>(f.createOutputStream());
        if(outputStream == nullptr)
        {
            throw std::runtime_error("The output stream cannot be allocated");
        }

        auto audioFormatWriter = std::unique_ptr<juce::AudioFormatWriter>(audioFormat->createWriterFor(outputStream.get(), sampleRate, static_cast<unsigned int>(numChannels), bitDepth, {}, 0));
        if(audioFormatWriter == nullptr)
        {
            throw std::runtime_error("The format writer cannot be allocated");
        }

        outputStream.release();

        if(mThread.joinable())
        {
            mThread.join();
        }

        mThread = std::thread([this, sliceStart, sliceEnd, blockSize, writer = std::move(audioFormatWriter)]()
                              {
                                  run(writer, {sliceStart, sliceEnd}, blockSize);
                              });
    }
}

void SliceExporter::stopExport()
{
}

void SliceExporter::run(std::unique_ptr<juce::AudioFormatWriter> const& writer, juce::Range<int64_t> const& sampleRange, const int blockSize)
{
    std::unique_lock<std::mutex> lock(mMutex);

    juce::AudioBuffer<float> buffer(static_cast<int>(writer->getNumChannels()), blockSize);
    juce::AudioSourceChannelInfo bufferToFill(buffer);

    auto readPosition = sampleRange.getStart();
    while(readPosition < sampleRange.getEnd())
    {
        auto numThisTime = std::min(blockSize, static_cast<int>(sampleRange.getEnd() - readPosition));
        if(!writer->writeFromAudioSampleBuffer(mReadBuffer, static_cast<int>(readPosition), numThisTime))
        {
            // gone wrong.
        }

        readPosition += numThisTime;
    }
}
