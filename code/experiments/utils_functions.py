import numpy as np
import os
import copy
from scipy.io.wavfile import read, write
import matplotlib.pyplot as plt
import math

def generateSineSignal(frequency = 220.0, length=1.0, fs=44100.0):
    '''
    length in seconds
    fs = sample rate
    '''

    fs = float(fs)
    samples = np.arange(length * fs) / fs
    signal = np.sin(2 * np.pi * frequency * samples)
    return signal

def triangleFn(t, p):
    '''
    Triangle wave function spanning range [-1,1]
    for time t and period p

    x(t) = 2 * | 2(t/p - flr(t/p + 1/2)) | - 1
    (note: period is just 1/frequency)
    '''
    return 2 * abs(2 * (t/p - math.floor(t/p + 1/2))) -  1

def generateTriangleSignal(frequency=220.0, length=1.0, fs=44100.0):
    period = 1/frequency

    num_samples = int(length * fs)
    signal = []
    for i in range(num_samples):
        t=i/fs
        val = triangleFn(t, period)
        signal.append(val)

    return np.array(signal)

def generateSquareSignal(frequency=220.0, length=1.0, fs=44100.0):
    num_samples = int(length*fs)
    signal = []
    for i in range(num_samples):
        t=i/fs
        signal.append(2 * (2 * math.floor(frequency*t) - math.floor(2*frequency*t)) + 1)

    return np.array(signal)

def generateSawtoothSignal(frequency=220.0, length=1.0, fs=44100.0):
    p = 1/frequency
    num_samples = int(length*fs)
    signal = []
    for i in range(num_samples):
        t=i/fs
        signal.append(2 * (2 * (t/p - math.floor(t/p + 1/2))))

    return np.array(signal)

def generateInverseSawtoothSignal(frequency=220.0, length=1.0, fs=44100.0):
    return -1*generateSawtoothSignal(frequency, length, fs)

def generateRandomSignal():
    pass

def generateRampEnvelope(minimum_amplitude = 0.0, maximum_amplitude = 1.0, signal_length = 1.0, sample_rate = 44100.0):
    return np.linspace(minimum_amplitude, maximum_amplitude, int(1 / signal_length * sample_rate))

INT16_FAC = (2**15)-1
INT32_FAC = (2**31)-1
INT64_FAC = (2**63)-1
norm_factor = {'int16':INT16_FAC, 'int32':INT32_FAC, 'int64':INT64_FAC, 'float32':1.0, 'float64':1.0}  

def wavread(filepath):
    """
    Read a wav file and convert it to a normalized floating point array
    filepath: path to file on disk to read (mono or stereo)
    returns sample_rate of file, x: floating point array, num_channels: number of channels of audio data
    """

    if(os.path.isfile(filepath) == False):
        raise ValueError("Invalid input filepath", filepath)

    sample_rate, data = read(filepath)

    # Scale and convert audio into range -1 to 1
    if len(data.shape) == 1 or len(data.shape) == 2:
        data = np.float32(data) / norm_factor[data.dtype.name]
    else:
        raise ValueError("Audio file should be mono or stereo, not ", len(data.shape), " channels")

    return sample_rate, data, 1 if len(data.shape) == 1 else data.shape[1]


def wavwrite(y, sample_rate, filepath):
    """
    Write a wav file of floating point data (y) in the range [-1, 1] by converting it to int16 using sample_rate
    and filepath
    """

    x = copy.deepcopy(y)
    x *= INT16_FAC
    x = np.int16(x)

    write(filepath, sample_rate, y)

def signal_plot(nrows, ncols, n, *xy, title="Signal"):
    plt.subplot(nrows, ncols, n)
    
    plt.plot(*xy)
    # if len(xy) == 2:
        
    # else:
    #     plt.plot(xy)

    plt.legend()
    plt.title(title)
    plt.grid()
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")


def magnitude_spectrum_plot(nrows, ncols, n, frequency_data, amplitude_data, title="Spectrum"):
    plt.subplot(nrows, ncols, n)
    plt.plot(frequency_data, amplitude_data)
    plt.title(title)
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude")


def spectrum_plot(nrows, ncols, n, frequency_data, amplitude_data, title="Spectrum"):
    plt.subplot(nrows, ncols, n)
    plt.plot(frequency_data, amplitude_data)
    plt.title(title)
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(amplitude_data)])


def specgram_plot(nrows, ncols, n, input_signal, sample_rate, title="Specgram"):
    plt.subplot(nrows, ncols, n)
    plt.specgram(input_signal, NFFT=256, Fs=sample_rate)
    plt.xlabel("Time [s]")
    plt.ylabel("Frequency [Hz]")


def next_power_of_2(x):
    return 1 if x == 0 else int(2**np.ceil(np.log2(x)))
