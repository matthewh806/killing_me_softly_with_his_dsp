import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq, rfft, rfftfreq

def db_fft(in_data, sample_rate):
    """
    Performs a real FFT on the data supplied, uses a hanning window to weight the data appropriately
    in_data: 1 dimensional normalized ([-1, 1]) numpy array of data, sample_rate: sample rate of the input audio

    Note: Creates a copy of the in_data for the purposes of windowing so that we don't modify the input data
    This is less efficient than reversing the window operation - but at least we know the in_data is totally unmodified

    returns:
        frequencies: array values based on bin centers calculated using the sample rate
        magnitude_dbs: magnitude array of the fft data calculated using 10*log10(magnitude spectrum)

        The returned array sizes are len(data) // 2 + 1

    """
    data = np.array(in_data, copy=True)
    data_length = len(data)
    weighting = np.hanning(data_length)
    data *= weighting

    if not np.all(np.isfinite(data)):
        np.nan_to_num(data, copy=False, nan=0.0, posinf=0.0, neginf=0.0)

    fft_values = rfft(data)
    frequencies = rfftfreq(data_length, d=1/sample_rate)
    magnitude_spectrum = np.abs(fft_values) * 2 / np.sum(weighting)
    magnitude_dbs = 10*np.log10(magnitude_spectrum)

    return frequencies, magnitude_dbs

if __name__ == "__main__":
    N = 600 # no sample points.
    T = 1.0 / 800.0 # sample spacing
    x = np.linspace(0.0, N*T, N, endpoint=False)
    y = np.sin(5.0*2.0*np.pi*x) + 0.5*np.sin(80.0*2.0*np.pi*x)

    yf = fft(y)
    xf = fftfreq(N, T)[:N//2]

    plt.plot(xf, 2.0/N * np.abs(yf[0:N//2]))
    plt.show()



    

