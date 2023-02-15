#!/bin/sh

# This script generates simple audio wav files for testing using the SoX library

ThisPath="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
TEST_AUDIO_PATH="$ThisPath/../test/audio"

echo '\033[0;34m' "Preparing to generate test audio files"
echo '\033[0m'

sox -r 44100 -b 16 -n "$TEST_AUDIO_PATH/44100Hz_1f_silence.wav"     trim 0 1
sox -r 44100 -b 16 -n "$TEST_AUDIO_PATH/44100Hz_1s_sine440.wav"     synth 1 sine 440 vol 0.9
sox -r 44100 -b 16 -n "$TEST_AUDIO_PATH/44100Hz_10s_sine_sweep.wav" synth 10 sine 100-2000 vol 0.9
sox -r 44100 -b 16 -n "$TEST_AUDIO_PATH/44100Hz_5s_brown_noise.wav" synth 5 brownnoise vol 0.9
sox -r 44100 -b 16 -n "$TEST_AUDIO_PATH/44100Hz_1s_triangle220.wav" synth 1 triangle 220 vol 0.9

sox -r 22050 -b 16 -n "$TEST_AUDIO_PATH/22050Hz_1s_sine440.wav"     synth 1 sine 440 vol 0.9
sox -r 8000 -b 16 -n  "$TEST_AUDIO_PATH/8000Hz_1s_sine440.wav"      synth 1 sine 440 vol 0.9
sox -r 1000 -b 16 -n  "$TEST_AUDIO_PATH/1000Hz_1s_sine440.wav"      synth 1 sine 440 vol 0.9
sox -r 48000 -b 16 -n "$TEST_AUDIO_PATH/48000Hz_1s_sine440.wav"     synth 1 sine 440 vol 0.9
sox -r 96000 -b 16 -n "$TEST_AUDIO_PATH/96000Hz_1s_sine440.wav"     synth 1 sine 440 vol 0.9

echo '\033[0;34m' "Done"
echo '\033[0m'
