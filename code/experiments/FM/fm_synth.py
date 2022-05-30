import matplotlib.pyplot as plt
import numpy as np
from scipy.fft import fft, fftfreq
from scipy.io.wavfile import write
import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF
import math

def generateFMSignal(carrier_frequency, carrier_envelope, modulator_frequency, modulator_envelope, signal_length=1.0, sample_rate=44100.0):
    samples = np.arange(signal_length * float(sample_rate)) / float(sample_rate)
    frequency_deviation = modulator_freq * modulator_envelope
    modulator = frequency_deviation * UF.generateSineSignal(modulator_frequency, signal_length, sample_rate)
    return carrier_envelope * np.sin(2 * np.pi * carrier_frequency * samples + modulator)

if __name__ == "__main__":
    carrier_freq = 300.0
    modulator_freq = 600.0
    signal_length = 1.0
    sampling_rate = 44100.0
    carrier_envelope = UF.generateRampEnvelope(0.0, 1.0, signal_length, sampling_rate)
    modulator_envelope = UF.generateRampEnvelope(1/carrier_freq, 1.0, signal_length, sampling_rate)
    fm_signal = generateFMSignal(carrier_freq, carrier_envelope, modulator_freq, modulator_envelope, signal_length, sampling_rate)

    print("fm signal: ", fm_signal)
    write("fm_snyth.wav", int(sampling_rate), fm_signal)
    num_samples = int(signal_length * sampling_rate)
    sample_spacing = 1/sampling_rate

    print("Num samples: ", num_samples, ", sample spacing: ", sample_spacing)
    yf = fft(fm_signal)
    xf = fftfreq(num_samples, sample_spacing)[:num_samples//2]

    fig = plt.figure()
    ax1 = fig.add_subplot(311)
    ax1.set_title("FM signal")
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Amplitude")
    ax1.plot(fm_signal)
    ax3 = fig.add_subplot(313)
    ax3.set_title("FFT plot")
    ax3.set_xlabel("Frequency")
    ax3.set_ylabel("Amplitude")
    ax3.plot(xf, 2.0/num_samples * np.abs(yf[:num_samples//2]))
    plt.show()