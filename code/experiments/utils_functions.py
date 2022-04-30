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