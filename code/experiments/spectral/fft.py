import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import rfft, rfftfreq

'''
TODO: Allow custom window type to be specified as an argument
'''

def mag_fft(in_data, sample_rate, oversamp=0):
    """
    Performs a real FFT on the data supplied, uses a hanning window to weight the data appropriately
    in_data: 1 dimensional normalized ([-1, 1]) numpy array of data, sample_rate: sample rate of the input audio

    oversamp determines the the amount of zero padding that will be added to the in_data and passed to the FFT routine
    0 defaults to no zero padding, 1 will round to the nearest power of two above len(in_data), 2 will round to the 
    next power of two above len(in_data) etc. This improves the accuracy of the output data at the cost of being more
    computationally expensive

    Note: Creates a copy of the in_data for the purposes of windowing so that we don't modify the input data
    This is less efficient than reversing the window operation - but at least we know the in_data is totally unmodified

    returns:
        frequencies: array values based on bin centers calculated using the sample rate
        magnitude_dbs: magnitude array of the fft data

        The returned array sizes are len(data) // 2 + 1

    """
    data = np.array(in_data, copy=True)
    data_length = len(data)
    weighting = np.hanning(data_length)
    data *= weighting

    if not np.all(np.isfinite(data)):
        np.nan_to_num(data, copy=False, nan=0.0, posinf=0.0, neginf=0.0)

    if oversamp > 0:
        zero_pad_len = UF.next_power_of_2(len(in_data))
        data_length = zero_pad_len * oversamp

    fft_values = rfft(data, n=data_length)
    frequencies = rfftfreq(data_length, d=1/sample_rate)
    magnitude_spectrum = np.abs(fft_values) * 2 / np.sum(weighting)

    return frequencies, magnitude_spectrum


def db_fft(in_data, sample_rate, oversamp=0):
    """
    Performs a real FFT on the data supplied, uses a hanning window to weight the data appropriately
    in_data: 1 dimensional normalized ([-1, 1]) numpy array of data, sample_rate: sample rate of the input audio

    oversamp determines the the amount of zero padding that will be added to the in_data and passed to the FFT routine
    0 defaults to no zero padding, 1 will round to the nearest power of two above len(in_data), 2 will round to the 
    next power of two above len(in_data) etc. This improves the accuracy of the output data at the cost of being more
    computationally expensive

    Note: Creates a copy of the in_data for the purposes of windowing so that we don't modify the input data
    This is less efficient than reversing the window operation - but at least we know the in_data is totally unmodified

    returns:
        frequencies: array values based on bin centers calculated using the sample rate
        magnitude_dbs: magnitude array of the fft data calculated using 10*log10(magnitude spectrum)

        The returned array sizes are len(data) // 2 + 1

    """
    frequencies, magnitude_spectrum = mag_fft(in_data, sample_rate, oversamp)
    return frequencies, 10*np.log10(magnitude_spectrum)


if __name__ == "__main__":
    N = 600 # no sample points.
    T = 1.0 / 800.0 # sample spacing
    x = np.linspace(0.0, N*T, N, endpoint=False)
    y = np.sin(5.0*2.0*np.pi*x) + 0.5*np.sin(80.0*2.0*np.pi*x)

    xf, yf = db_fft(y, 800)

    plt.plot(xf, yf)
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude [dB]")
    plt.show()



    

