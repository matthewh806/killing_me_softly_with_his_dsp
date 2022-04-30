import matplotlib.pyplot as plt
import numpy as np
from scipy.fft import fft, fftfreq
from scipy.io.wavfile import write
import math

def generateSineSignal(frequency = 220.0, length=1.0, fs=44100.0):
    '''
    length in seconds
    fs = sample rate
    '''

    fs = float(fs)
    samples = np.arange(length * fs) / fs
    print("samples: {}".format(samples))
    signal = np.sin(2 * np.pi * frequency * samples)

    print("signal: {}".format(signal))
    return signal

def generateFMSignal(carrier_frequency = 220.0, carrier_amplitude = 1.0, modulator_frequency = 220.0, modulation_index = 1.0, signal_length=1.0, sample_rate=44100.0):
    samples = np.arange(signal_length * float(sample_rate)) / float(sample_rate)
    modulator = modulation_index * generateSineSignal(modulator_frequency, signal_length, sample_rate)
    return carrier_amplitude * np.sin(2 * np.pi * carrier_frequency * samples + modulator)

if __name__ == "__main__":
    carrier_freq = 300.0
    modulator_freq = 2.9 * carrier_freq / 4.75
    frequency_deviation = abs(carrier_freq - modulator_freq)
    modulation_index = frequency_deviation / modulator_freq
    signal_length = 1.0
    sampling_rate = 44100.0
    fm_signal = generateFMSignal(carrier_freq, 1.0, modulator_freq, modulation_index, signal_length, sampling_rate)

    print("fm signal: ", fm_signal)
    write("fm_snyth.wav", int(sampling_rate), fm_signal)
    num_samples = int(signal_length * sampling_rate)
    sample_spacing = 1/sampling_rate

    print("Num samples: ", num_samples, ", sample spacing: ", sample_spacing)
    yf = fft(fm_signal)
    xf = fftfreq(num_samples, sample_spacing)[:num_samples//2]

    fig = plt.figure()
    ax1 = fig.add_subplot(211)
    ax1.set_title("FM signal")
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Amplitude")
    ax1.plot(fm_signal)
    ax2 = fig.add_subplot(212)
    ax2.set_title("FFT plot")
    ax2.set_xlabel("Frequency")
    ax2.set_ylabel("Amplitude")
    ax2.plot(xf, 2.0/num_samples * np.abs(yf[:num_samples//2]))
    plt.show()