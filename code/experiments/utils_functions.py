import numpy as np
import os
import copy
from scipy.io.wavfile import read, write
import matplotlib.pyplot as plt
import math

class CircularBuffer:
    '''
    Very basic fixed size circular buffer
    Initialises the data array to size 
    and fills with zeros
    '''

    def __init__(self, size):
        self.size = size
        self.head = 0 # write pointer
        self.tail = 0 # read pointer
        self.data = np.zeros(size)

    def clear(self):
        self.data = np.zeros(self.size)
        self.head = 0
        self.read = 0

    def write(self, value):
        self.data[self.head] = value
        self.head = (self.head + 1) % self.size

    def read(self):
        value = self.data[self.tail]
        self.tail = (self.tail + 1) % self.size
        return value


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

def autocorrelation(x, tapered=True):
    '''
    inputs:
        x: 1 dimensional np array
        tapered: Determines if the returned result is biased (towards earlier)
                 peaks or not

    Computes:
        r(k) = sum_n (x(n) * x(n-k))

        where r is the autocorrelation of the signal x with the k parameter provided
        The function is maximused when k = 0 
        Symmetric about k

        The lag time (k) will be based on the length of the signal len(x)

    returns:
        1 dimensional autocorrelation function array r(k) of length k

    '''
    
    signal_length = len(x)
    r = np.zeros(signal_length)

    for k in range(signal_length):
        r_k = 0.0
        for n in range(k, signal_length):
            r_k += x[n] * x[n-k]

        r[k] = r_k if tapered else r_k / (signal_length-k)

    return r

def peak_detection(x, threshold=0.1):
    '''
    Finds the peaks in an array above a given threshold value 

    Note: its best to normalise the peak amplitudes to be
          between [-1,1]

    inputs:
        x: the input signal array to determine the peaks of
        threshold: The minimum value to be considered a peak.

    computes:
        Finds local maxima by searching for points of inflection
        (where the signs either side of x[i] are different)

        For the first sample x[-1] is out of range so we assume
        x[0] is bigger
        For the final sample x[+1] is out of range so we 
        assume x[len(x)-1] is bigger

    outputs:
        Two equally sized 1D numpy arrays containing
        peak positions & peak amplitudes
    '''

    positions = np.array([]) 
    peaks = np.array([])
    signal_len = len(x)

    for i in range(signal_len):
        bigger_than_prev = x[i-1] < x[i] if i > 0 else True
        bigger_than_next = x[i+1] < x[i] if i < signal_len-1 else True

        if bigger_than_prev and bigger_than_next and x[i] >= threshold:
            positions = np.append(positions, i)
            peaks = np.append(peaks, x[i])

    return positions, peaks

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
    plt.legend()
    plt.title(title)
    plt.grid()
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")

def fft_plot_data(signal, fslice=slice(0,100), sample_rate=44100):
    fd = np.fft.fft(signal)
    fd_mag = np.abs(fd)
    x = np.linspace(0, sample_rate, len(signal))
    y = fd_mag * 2 / sample_rate

    return x[fslice], y[fslice]

def magnitude_spectrum_plot(nrows, ncols, n, frequency_data, amplitude_data, title="Spectrum", xticks=None, yticks=None):
    plt.subplot(nrows, ncols, n)
    plt.plot(frequency_data, amplitude_data)
    plt.title(title)
    plt.grid()
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude")

    if xticks is not None: plt.xticks(xticks)
    if yticks is not None: plt.yticks(yticks)


def spectrum_plot(nrows, ncols, n, frequency_data, amplitude_data, title="Spectrum"):
    plt.subplot(nrows, ncols, n)
    plt.plot(frequency_data, amplitude_data)
    plt.title(title)
    plt.grid()
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(amplitude_data)])


def specgram_plot(nrows, ncols, n, input_signal, sample_rate, title="Specgram"):
    plt.subplot(nrows, ncols, n)
    plt.specgram(input_signal, NFFT=256, Fs=sample_rate)
    plt.xlabel("Time [s]")
    plt.ylabel("Frequency [Hz]")


def get_sigspectrum(signal, fslice=slice(0, 100), sample_rate=44100, figsize=(25,9), sig_title="Signal", spec_title="Spectrum", xsticks=None, ysticks=None):
    fig, axes = plt.subplots(nrows=2, ncols=1, figsize=figsize)

    sig_ax = axes[0]
    sig_ax.plot(signal)
    sig_ax.legend()
    sig_ax.set_title(sig_title)
    sig_ax.grid()
    sig_ax.set_xlabel("Time [s]")
    sig_ax.set_ylabel("Amplitude")

    fx, fy = fft_plot_data(signal, fslice=fslice)

    spec_ax = axes[1]
    spec_ax.plot(fx, fy)
    spec_ax.set_title(spec_title)
    spec_ax.grid()
    spec_ax.set_xlabel("Frequency [Hz]")
    spec_ax.set_ylabel("Amplitude")

    if xsticks is not None: spec_ax.set_xticks(xsticks)
    if ysticks is not None: spec_ax.set_yticks(ysticks)

    return fig

def plot_sigspectrum(signal, fslice=slice(0, 100), sample_rate=44100, figsize=(25,9), sig_title="Signal", spec_title="Spectrum", xsticks=None, ysticks=None):
    '''
    A simple convenience plotter for displaying both
    a signal & spectrum together in one figure

    The FFT of the signal is calculated internally by this method

    xsticks & ysticks are tick value lists for the spectrum plot
    '''
    get_sigspectrum(signal, fslice, sample_rate, figsize, sig_title, spec_title, xsticks, ysticks)
    plt.show()


def next_power_of_2(x):
    return 1 if x == 0 else int(2**np.ceil(np.log2(x)))
