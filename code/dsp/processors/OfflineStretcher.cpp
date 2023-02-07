#include "OfflineStretcher.h"

using namespace OUS;

OfflineStretchProcessor::OfflineStretchProcessor(TemporaryFile& file, juce::AudioSampleBuffer& stretchSrc, float stretchFactor, float pitchFactor, double sampleRate, std::function<void()> onThreadComplete)
: juce::ThreadWithProgressWindow("Offline stretcher", true, false)
, mFile(file)
, mStretchSrc(stretchSrc)
, mStretchFactor(stretchFactor)
, mPitchShiftFactor(pitchFactor)
, mOnThreadComplete(onThreadComplete)
{
    std::unique_ptr<RubberBand::RubberBandStretcher> newBand(new RubberBand::RubberBandStretcher(static_cast<size_t>(sampleRate), static_cast<size_t>(2)));
    mRubberBandStretcher.reset(newBand.release());
}

void OfflineStretchProcessor::run()
{
    auto const srcBufferLength = static_cast<size_t>(mStretchSrc.getNumSamples());
    auto const srcChannels = static_cast<size_t>(mStretchSrc.getNumChannels());
    if(!mRubberBandStretcher || srcBufferLength <= 0 || srcChannels <= 0)
    {
        return;
    }

    mRubberBandStretcher->reset();
    mRubberBandStretcher->setTimeRatio(mStretchFactor);
    mRubberBandStretcher->setPitchScale(mPitchShiftFactor);

    // 1. phase 1 is study
    size_t sample = 0;
    size_t percent = 0;

    std::cout << "Phase 1: Studying " << srcBufferLength << " samples\n";

    auto constexpr bufferSize = static_cast<size_t>(1024);
    while(sample < srcBufferLength)
    {
        auto const samplesThisTime = std::min(bufferSize, srcBufferLength - (sample));

        juce::AudioSampleBuffer readBuffer(static_cast<int>(srcChannels), static_cast<int>(samplesThisTime));
        for(size_t ch = 0; ch < srcChannels; ++ch)
        {
            readBuffer.copyFrom(static_cast<int>(ch), 0, mStretchSrc, static_cast<int>(ch), static_cast<int>(sample), static_cast<int>(samplesThisTime));
        }

        auto const finalSamples = sample + bufferSize >= srcBufferLength;

        std::cout << "Studying " << samplesThisTime << " samples\n";
        if(finalSamples)
        {
            std::cout << " final frames\n";
        }
        mRubberBandStretcher->study(readBuffer.getArrayOfReadPointers(), samplesThisTime, finalSamples);

        auto const p = static_cast<size_t>(static_cast<double>(sample) * 100.0 / static_cast<double>(srcBufferLength));
        if(p > percent || sample == 0)
        {
            percent = p;
            std::cout << "\r" << percent << "%\n";
            setProgress(0.5 * static_cast<double>(percent)); // this is the first half of the overall
        }

        sample += samplesThisTime;
    }

    std::cout << "Phase 1: Studying finished\n";

    // 2. phase 2 is stretch
    sample = 0;
    percent = 0;

    std::cout << "Phase 2: Stretch Armstrong " << srcBufferLength << " samples\n";

    // Create buffer based on estimated size...
    auto stretchedBufferSize = static_cast<int>(srcBufferLength * mStretchFactor);
    std::cout << "Estimated output buffer size: " << stretchedBufferSize << "\n";
    mStretchedBuffer.setSize(static_cast<int>(srcChannels), stretchedBufferSize, false, true);
    auto sampleOut = 0;

    while(sample < srcBufferLength)
    {
        auto const samplesThisTime = std::min(bufferSize, srcBufferLength - sample);
        std::cout << "Reading file position: " << sample << ", to " << sample + samplesThisTime << "\n";
        juce::AudioSampleBuffer readBuffer(static_cast<int>(srcChannels), static_cast<int>(samplesThisTime));
        for(size_t ch = 0; ch < srcChannels; ++ch)
        {
            readBuffer.copyFrom(static_cast<int>(ch), 0, mStretchSrc, static_cast<int>(ch), static_cast<int>(sample), static_cast<int>(samplesThisTime));
        }

        std::cout << "Processing " << samplesThisTime << " samples\n";
        auto const finalSamples = sample + bufferSize >= srcBufferLength;
        mRubberBandStretcher->process(readBuffer.getArrayOfReadPointers(), samplesThisTime, finalSamples);

        auto const available = mRubberBandStretcher->available();
        std::cout << "File buffer length: " << srcBufferLength << ", availableSamples: " << available << "\n";
        if(available > 0)
        {
            auto stretchedBuffer = juce::AudioBuffer<float>(static_cast<int>(srcChannels), available);
            mRubberBandStretcher->retrieve(stretchedBuffer.getArrayOfWritePointers(), static_cast<size_t>(available));

            if(sampleOut + available > stretchedBufferSize)
            {
                // we need to grow the buffer a bit
                std::cout << "Increasing buffer size: " << stretchedBufferSize << " (orig) to " << (stretchedBufferSize + sampleOut + available) << "\n";
                mStretchedBuffer.setSize(static_cast<int>(srcChannels), (stretchedBufferSize + sampleOut + available), true);
            }

            std::cout << "Writing to stretched buffer from position " << sampleOut << ", to " << sampleOut + available << "\n";
            for(size_t ch = 0; ch < srcChannels; ++ch)
            {
                for(int i = 0; i < available; ++i)
                {
                    auto value = std::max(-1.0f, std::min(1.0f, stretchedBuffer.getSample(static_cast<int>(ch), i)));
                    mStretchedBuffer.addSample(static_cast<int>(ch), sampleOut + i, value);
                }
            }

            sampleOut += available;
        }

        auto const p = static_cast<size_t>(static_cast<double>(sample) * 100.0 / static_cast<double>(srcBufferLength));
        if(p > percent || sample == 0)
        {
            percent = static_cast<size_t>(p);
            std::cout << "\r" << percent << "%\n";

            setProgress(0.5 + 0.5 * static_cast<double>(percent)); // this is second half of the overall
        }

        sample += samplesThisTime;
    }

    std::cout << "Phase 2: Stretch Armstrong finished\n";

    std::cout << "Phase 3: Remaining samples\n";
    std::cout << "Num" << mRubberBandStretcher->available() << "\n";
    while(mRubberBandStretcher->available() >= 0)
    {
        auto const availableSamples = mRubberBandStretcher->available();

        jassert(availableSamples >= 0);

        std::cout << "Completing: number remaining: " << availableSamples << "\n";
        auto stretchedBuffer = juce::AudioBuffer<float>(static_cast<int>(srcChannels), availableSamples);
        mRubberBandStretcher->retrieve(stretchedBuffer.getArrayOfWritePointers(), static_cast<size_t>(availableSamples));

        if(sampleOut + availableSamples > stretchedBufferSize)
        {
            // we need to grow the buffer a bit
            std::cout << "Increasing buffer size: " << stretchedBufferSize << " (orig) to " << (stretchedBufferSize + sampleOut + availableSamples) << "\n";
            mStretchedBuffer.setSize(static_cast<int>(srcChannels), (stretchedBufferSize + sampleOut + availableSamples), true);
        }

        std::cout << "Writing to stretched buffer from position " << sampleOut << ", to " << sampleOut + availableSamples << "\n";
        for(size_t ch = 0; ch < srcChannels; ++ch)
        {
            for(int i = 0; i < availableSamples; ++i)
            {
                auto value = std::max(-1.0f, std::min(1.0f, stretchedBuffer.getSample(static_cast<int>(ch), i)));
                mStretchedBuffer.addSample(static_cast<int>(ch), sampleOut + i, value);
            }
        }

        sampleOut += availableSamples;
    }
    std::cout << "Phase 3: Remaining samples complete\n";

    // save to temp file.
    // assume wav for now
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;

    if(auto fileStream = std::unique_ptr<FileOutputStream>(mFile.getFile().createOutputStream()))
    {
        if(fileStream->openedOk())
        {
            fileStream->setPosition(0);
            fileStream->truncate();
        }

        writer.reset(format.createWriterFor(fileStream.release(),
                                            44100.0,
                                            static_cast<unsigned int>(mStretchedBuffer.getNumChannels()),
                                            24,
                                            {},
                                            0));
        if(writer != nullptr)
        {
            writer->writeFromAudioSampleBuffer(mStretchedBuffer, 0, mStretchedBuffer.getNumSamples());
        }
    }
}

void OfflineStretchProcessor::threadComplete(bool userPressedCancel)
{
    if(mOnThreadComplete != nullptr)
    {
        mOnThreadComplete();
    }
}
