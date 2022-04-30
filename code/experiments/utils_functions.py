import numpy as np

def generateSineSignal(frequency = 220.0, length=1.0, fs=44100.0):
    '''
    length in seconds
    fs = sample rate
    '''

    fs = float(fs)
    samples = np.arange(length * fs) / fs
    signal = np.sin(2 * np.pi * frequency * samples)
    return signal

def generateRampEnvelope(minimum_amplitude = 0.0, maximum_amplitude = 1.0, signal_length = 1.0, sample_rate = 44100.0):
    return np.linspace(minimum_amplitude, maximum_amplitude, int(1 / signal_length * sample_rate))