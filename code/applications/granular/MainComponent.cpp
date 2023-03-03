#include "MainComponent.h"

using namespace OUS;

//==============================================================================

GranularWaveform::GranularWaveform(juce::AudioFormatManager& formatManager)
: WaveformAndRuler(formatManager)
{
    setZoomable(false);
}

void GranularWaveform::paint(juce::Graphics& g)
{
    WaveformAndRuler::paint(g);

    // draw the grains
    auto const lengthInSeconds = getTotalRange().getLength();
    auto const lengthInSamples = static_cast<size_t>(std::floor(lengthInSeconds * 44100.0));
    auto const waveformBounds = getLocalBounds();

    for(size_t i = 0; i < Scheduler::POOL_SIZE; ++i)
    {
        if(std::get<0>(mGrainInfo[i]) == false)
        {
            continue;
        }

        auto const samplePos = std::get<1>(mGrainInfo[i]);
        // convert from sample pos to screen pos

        auto const screenPos = static_cast<float>(samplePos) / lengthInSamples * static_cast<float>(waveformBounds.getWidth()) + static_cast<float>(waveformBounds.getX());
        g.setColour(std::get<3>(mGrainInfo[i]));
        g.drawEllipse(screenPos, std::get<2>(mGrainInfo[i]), 2, 2, 3);
    }
}

void GranularWaveform::updateGrainInfo(std::array<Grain, Scheduler::POOL_SIZE> const& grains)
{
    auto& random = juce::Random::getSystemRandom();
    for(size_t i = 0; i < Scheduler::POOL_SIZE; ++i)
    {
        if(std::get<0>(mGrainInfo[i]) != !grains[i].isGrainComplete())
        {
            auto const waveformHeight = getWaveform().getThumbnailBounds().getHeight();
            auto const waveformY = getWaveform().getThumbnailBounds().getY();
            std::get<2>(mGrainInfo[i]) = static_cast<float>(random.nextInt(waveformHeight)) + static_cast<float>(waveformY);

            juce::Colour colour(random.nextInt(juce::Range<int>(100, 256)),
                                random.nextInt(juce::Range<int>(50, 200)),
                                200);
            std::get<3>(mGrainInfo[i]) = colour;
        }

        std::get<0>(mGrainInfo[i]) = !grains[i].isGrainComplete();
        std::get<1>(mGrainInfo[i]) = grains[i].getGrainPosition();
    }

    repaint();
}

//==============================================================================
MainComponent::MainComponent(juce::AudioDeviceManager& activeDeviceManager)
: juce::AudioAppComponent(activeDeviceManager)
, juce::Thread("backgroundthread")
, mGrainDensitySlider("Grain density", "g/s")
, mGrainLengthSlider("Grain length", "ms")
, mGrainPositionRandomnessSlider("Position randomness", "")
, mGrainCountLabel("# grains:", "", 0, false)
, mSourceTypeSlider("Source type")
, mEnvelopeTypeSlider("Envelope type")
, mFrequencySlider("Frequency", "Hz")
, mGrainAmplitudeSlider("Grain Amplitude", "")
, mEnvelopeAttackSlider("Attack", "ms")
, mEnvelopeReleaseSlider("Release", "ms")
{
    mFormatManager.registerBasicFormats();

    addAndMakeVisible(mGrainCountLabel);
    mGrainCountLabel.setNumberOfDecimals(0);

    addAndMakeVisible(mGrainDensitySlider);
    mGrainDensitySlider.setRange({1.0, 100.0}, 1.0);
    mGrainDensitySlider.mLabels.add({0.0, "1"});
    mGrainDensitySlider.mLabels.add({1.0, "100"});
    mGrainDensitySlider.setValue(1.0);
    mGrainDensitySlider.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            mScheduler->setGrainDensity(mGrainDensitySlider.getValue());
        }
    };

    addAndMakeVisible(mGrainLengthSlider);
    mGrainLengthSlider.setRange({10.0, 1000.0}, 1.0);
    mGrainLengthSlider.mLabels.add({0.0, "10"});
    mGrainLengthSlider.mLabels.add({1.0, "1000"});
    mGrainLengthSlider.setValue(300.0); // try to avoid killing our ears on startup
    mGrainLengthSlider.onValueChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            auto const lengthSeconds = mGrainLengthSlider.getValue() / 1000.0;
            mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));
        }
    };

    addAndMakeVisible(mGrainPositionRandomnessSlider);
    mGrainPositionRandomnessSlider.setRange({0.0, 1.0}, 0.05);
    mGrainPositionRandomnessSlider.mLabels.add({0.0, "0.0"});
    mGrainPositionRandomnessSlider.mLabels.add({1.0, "1.0"});
    mGrainPositionRandomnessSlider.setValue(0.0);
    mGrainPositionRandomnessSlider.onValueChange = [this]()
    {
        if(mScheduler == nullptr)
        {
            return;
        }

        mScheduler->setPositionRandomness(mGrainPositionRandomnessSlider.getValue());
    };

    addAndMakeVisible(mSourceTypeSlider);
    mSourceTypeSlider.comboBox.addItem("Sample", 1);
    mSourceTypeSlider.comboBox.addItem("Synthetic", 2);
    mSourceTypeSlider.comboBox.setSelectedId(1);
    mSourceTypeSlider.comboBox.onChange = [this]()
    {
        auto const sourceType = static_cast<Source::SourceType>(mSourceTypeSlider.comboBox.getSelectedItemIndex());
        if(sourceType == mSourceType)
        {
            return;
        }

        mSourceType = sourceType;

        if(mScheduler != nullptr)
        {
            mScheduler->shouldSynthesise = false;
        }
        mScheduler = nullptr;
        switch(mSourceType)
        {
            case Source::SourceType::sample:
            {
                mWaveformComponent.setVisible(true);
                mFrequencySlider.setVisible(false);
                mGrainPositionRandomnessSlider.setVisible(true);
            }
            break;
            case Source::SourceType::synthetic:
            {
                mWaveformComponent.setVisible(false);
                mFrequencySlider.setVisible(true);
                mGrainPositionRandomnessSlider.setVisible(false);

                mScheduler = std::make_unique<Scheduler>();
                if(mScheduler == nullptr)
                {
                    // todo: throw error
                    std::cerr << "Scheduler could not be created!\n";
                    return;
                }

                auto srcEssence = std::make_unique<SinewaveSource::OscillatorEssence>();
                srcEssence->frequency = mFrequencySlider.getValue();

                mScheduler->setSourceEssence(std::move(srcEssence));
                mScheduler->prepareToPlay(mBlockSize, mSampleRate);
                mScheduler->setGrainDensity(mGrainDensitySlider.getValue());
                auto const lengthSeconds = mGrainLengthSlider.getValue() / 1000.0;

                auto const envelopeType = static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex());
                std::unique_ptr<Envelope::Essence> envEssence = nullptr;
                if(envelopeType == Envelope::EnvelopeType::trapezoidal)
                {
                    envEssence = std::make_unique<TrapezoidalEnvelope::TrapezoidalEssence>();
                    // todo:: mess improve
                    dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envEssence.get())->attackSamples = 1024;
                    dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envEssence.get())->releaseSamples = 1024;
                }
                else if(envelopeType == Envelope::EnvelopeType::parabolic)
                {
                    envEssence = std::make_unique<ParabolicEnvelope::ParabolicEssence>();
                }
                mScheduler->setEnvelopeEssence(std::move(envEssence));

                mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));
                mScheduler->shouldSynthesise = true;
            }
            break;
        }

        resized();
    };

    addAndMakeVisible(mEnvelopeTypeSlider);
    mEnvelopeTypeSlider.comboBox.addItem("Trapezoidal", 1);
    mEnvelopeTypeSlider.comboBox.addItem("Parabolic", 2);
    mEnvelopeTypeSlider.comboBox.setSelectedId(1);
    mEnvelopeTypeSlider.comboBox.onChange = [this]()
    {
        if(mScheduler != nullptr)
        {
            auto const envelopeType = static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex());
            std::unique_ptr<Envelope::Essence> envEssence = nullptr;
            if(envelopeType == Envelope::EnvelopeType::trapezoidal)
            {
                envEssence = std::make_unique<TrapezoidalEnvelope::TrapezoidalEssence>();
                // todo:: mess improve
                dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envEssence.get())->attackSamples = 1024;
                dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envEssence.get())->releaseSamples = 1024;

                mEnvelopeAttackSlider.setVisible(true);
                mEnvelopeReleaseSlider.setVisible(true);
            }
            else if(envelopeType == Envelope::EnvelopeType::parabolic)
            {
                envEssence = std::make_unique<ParabolicEnvelope::ParabolicEssence>();
                mEnvelopeAttackSlider.setVisible(false);
                mEnvelopeReleaseSlider.setVisible(false);
            }
            mScheduler->setEnvelopeEssence(std::move(envEssence));
        }
    };

    addAndMakeVisible(mWaveformComponent);
    mWaveformComponent.onNewFileDropped = [this](juce::String& path)
    {
        juce::String err;
        loadSample(path, err);
    };

    addChildComponent(mFrequencySlider);
    mFrequencySlider.setRange({65.4, 1046.502}, 1);
    mFrequencySlider.mLabels.add({0.0, "C2"});
    mFrequencySlider.mLabels.add({1.0, "C6"});
    mFrequencySlider.setValue(220.0);
    mFrequencySlider.onValueChange = [this]()
    {
        if(mScheduler == nullptr)
        {
            return;
        }

        auto essence = dynamic_cast<SinewaveSource::OscillatorEssence*>(mScheduler->getSourceEssence());
        if(essence == nullptr)
        {
            std::cout << "Could not cast Schedulers Essence to the expected type: SinewaveSource::OscillatorEssence\n";
            return;
        }

        essence->frequency = mFrequencySlider.getValue();
    };

    addAndMakeVisible(mGrainAmplitudeSlider);
    mGrainAmplitudeSlider.setRange({0.0, 1.0}, 0.05);
    mGrainAmplitudeSlider.mLabels.add({0.0, "0.0"});
    mGrainAmplitudeSlider.mLabels.add({1.0, "1.0"});
    mGrainAmplitudeSlider.setValue(0.6);
    mGrainAmplitudeSlider.onValueChange = [this]()
    {
        updateEnvelopeEssence();
    };

    addAndMakeVisible(mEnvelopeAttackSlider);
    mEnvelopeAttackSlider.setRange({0.0, 200}, 1.0);
    mEnvelopeAttackSlider.mLabels.add({0.0, "0.0"});
    mEnvelopeAttackSlider.mLabels.add({1.0, "200.0"});
    mEnvelopeAttackSlider.setValue(50);
    mEnvelopeAttackSlider.onValueChange = [this]()
    {
        updateEnvelopeEssence();
    };

    addAndMakeVisible(mEnvelopeReleaseSlider);
    mEnvelopeReleaseSlider.setRange({0.0, 200}, 1.0);
    mEnvelopeReleaseSlider.mLabels.add({0.0, "0.0"});
    mEnvelopeReleaseSlider.mLabels.add({1.0, "200.0"});
    mEnvelopeReleaseSlider.setValue(50);
    mEnvelopeReleaseSlider.onValueChange = [this]()
    {
        updateEnvelopeEssence();
    };

    setSize(600, 460);
    startTimer(200);
    setAudioChannels(2, 2);
}

MainComponent::~MainComponent()
{
    stopTimer();
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    mBlockSize = samplesPerBlockExpected;
    mSampleRate = static_cast<int>(sampleRate);
    mWaveformComponent.setSampleRate(static_cast<float>(sampleRate));

    if(mScheduler != nullptr)
    {
        mScheduler->prepareToPlay(mBlockSize, mSampleRate);
    }
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill)
{
    if(mScheduler != nullptr)
    {
        mScheduler->synthesise(bufferToFill.buffer, bufferToFill.numSamples);
    }
}

void MainComponent::releaseResources()
{
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds().reduced(20, 20);

    auto rotaryBounds = bounds.removeFromTop(100);
    auto const threeColumnSliderWidth = static_cast<int>(rotaryBounds.getWidth() * 0.30f);
    auto const spacingWidth = (rotaryBounds.getWidth() - threeColumnSliderWidth * 3) / 2;

    mGrainDensitySlider.setBounds(rotaryBounds.removeFromLeft(threeColumnSliderWidth));
    rotaryBounds.removeFromLeft(spacingWidth);
    mGrainLengthSlider.setBounds(rotaryBounds.removeFromLeft(threeColumnSliderWidth));
    rotaryBounds.removeFromLeft(spacingWidth);
    mGrainPositionRandomnessSlider.setBounds(rotaryBounds.removeFromLeft(threeColumnSliderWidth));

    bounds.removeFromTop(10);
    auto comboBoxBounds = bounds.removeFromTop(30);
    auto const twoColumnSliderWidth = static_cast<int>(comboBoxBounds.getWidth() * 0.48f);
    auto const twoColumnSpacingWidth = comboBoxBounds.getWidth() - twoColumnSliderWidth * 2;
    mSourceTypeSlider.setBounds(comboBoxBounds.removeFromLeft(twoColumnSliderWidth));
    comboBoxBounds.removeFromLeft(twoColumnSpacingWidth);
    mEnvelopeTypeSlider.setBounds(comboBoxBounds.removeFromLeft(twoColumnSliderWidth));

    bounds.removeFromTop(10);
    auto envelopeBounds = bounds.removeFromTop(100);
    mGrainAmplitudeSlider.setBounds(envelopeBounds.removeFromLeft(threeColumnSliderWidth));

    auto const envelopeType = static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex());
    if(envelopeType == Envelope::EnvelopeType::trapezoidal)
    {
        envelopeBounds.removeFromLeft(spacingWidth);
        mEnvelopeAttackSlider.setBounds(envelopeBounds.removeFromLeft(threeColumnSliderWidth));
        envelopeBounds.removeFromLeft(spacingWidth);
        mEnvelopeReleaseSlider.setBounds(envelopeBounds.removeFromLeft(threeColumnSliderWidth));
    }

    mGrainCountLabel.setBounds(bounds.removeFromTop(40));

    bounds.removeFromTop(10);
    if(mSourceType == Source::SourceType::sample)
    {
        mWaveformComponent.setBounds(bounds.removeFromTop(100));
    }
    else if(mSourceType == Source::SourceType::synthetic)
    {
        mFrequencySlider.setBounds(bounds.removeFromTop(100).removeFromLeft(threeColumnSliderWidth));
    }
}

bool MainComponent::loadSample(juce::String const& filePath, juce::String& error)
{
    juce::ignoreUnused(error);

    juce::File file(filePath);
    mReader = std::unique_ptr<juce::AudioFormatReader>(mFormatManager.createReaderFor(file));
    if(mReader == nullptr)
    {
        return false;
    }

    auto const numChannels = static_cast<int>(mReader.get()->numChannels);
    auto const numSamples = static_cast<int>(mReader.get()->lengthInSamples);
    ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer(file.getFileName(), numChannels, numSamples);
    mReader.get()->read(newBuffer->getAudioSampleBuffer(), 0, numSamples, 0, true, true);
    {
        const juce::SpinLock::ScopedLockType lock(mMutex);
        mCurrentBuffer = newBuffer;
    }

    mScheduler = std::make_unique<Scheduler>();
    if(mScheduler == nullptr)
    {
        // todo: throw error
        std::cerr << "Scheduler could not be created!\n";
        return false;
    }

    auto srcEssence = std::make_unique<SampleSource::SampleEssence>();
    srcEssence->audioSampleBuffer = mCurrentBuffer->getAudioSampleBuffer();
    srcEssence->position = 0.0;

    mScheduler->setSourceEssence(std::move(srcEssence));
    mScheduler->prepareToPlay(mBlockSize, mSampleRate);
    mScheduler->setGrainDensity(mGrainDensitySlider.getValue());
    auto const lengthSeconds = mGrainLengthSlider.getValue() / 1000.0;
    mScheduler->setGrainDuration(static_cast<size_t>(lengthSeconds * 44100.0));

    auto const envelopeType = static_cast<Envelope::EnvelopeType>(mEnvelopeTypeSlider.comboBox.getSelectedItemIndex());
    std::unique_ptr<Envelope::Essence> envEssence = nullptr;
    if(envelopeType == Envelope::EnvelopeType::trapezoidal)
    {
        envEssence = std::make_unique<TrapezoidalEnvelope::TrapezoidalEssence>();
        // todo:: mess improve
        dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envEssence.get())->attackSamples = 1024;
        dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(envEssence.get())->releaseSamples = 1024;
    }
    else if(envelopeType == Envelope::EnvelopeType::parabolic)
    {
        envEssence = std::make_unique<ParabolicEnvelope::ParabolicEssence>();
    }
    mScheduler->setEnvelopeEssence(std::move(envEssence));
    mScheduler->setPositionRandomness(mGrainPositionRandomnessSlider.getValue());

    mWaveformComponent.setThumbnailSource(mCurrentBuffer->getAudioSampleBuffer());

    mScheduler->shouldSynthesise = true;

    return true;
}

void MainComponent::run()
{
    while(!threadShouldExit())
    {
        checkForBuffersToFree();
        wait(500);
    }
}

void MainComponent::checkForBuffersToFree()
{
    for(auto i = mBuffers.size(); --i >= 0;)
    {
        ReferenceCountedBuffer::Ptr buffer(mBuffers.getUnchecked(i));

        if(buffer->getReferenceCount() == 2)
        {
            mBuffers.remove(i);
        }
    }
}

void MainComponent::timerCallback()
{
    if(mScheduler != nullptr)
    {
        auto const grainCount = mScheduler->getNumberOfGrains();
        mGrainCountLabel.setValue(grainCount, juce::NotificationType::sendNotificationAsync);

        mWaveformComponent.updateGrainInfo(mScheduler->getGrains());
    }
}

void MainComponent::updateEnvelopeEssence()
{
    if(mScheduler == nullptr)
    {
        return;
    }

    auto* essence = mScheduler->getEnvelopeEssence();

    if(essence == nullptr)
    {
        return;
    }

    essence->grainAmplitude = static_cast<float>(mGrainAmplitudeSlider.getValue());

    if(dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(essence))
    {

        /*
            convert time in ms -> length in samples
            length_in_samples = time_in_ms / 1000 * 44100
         */

        auto const attackSamples = mEnvelopeAttackSlider.getValue() / 1000.0 * 44100.0;
        auto const releaseSamples = mEnvelopeReleaseSlider.getValue() / 1000.0 * 44100.0;

        dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(essence)->attackSamples = static_cast<size_t>(std::floor(attackSamples));
        dynamic_cast<TrapezoidalEnvelope::TrapezoidalEssence*>(essence)->releaseSamples = static_cast<size_t>(std::floor(releaseSamples));
    }
}
