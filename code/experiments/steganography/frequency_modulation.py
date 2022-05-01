import matplotlib.pyplot as plt
import numpy as np
from scipy.fft import fft, fftfreq, rfft, rfftfreq
import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF
from filters import lowpass

"""
Note: Only handle mono files for the time being

TODO: Check the filtering operation when cutoff ~300Hz causes inf values and a messed up FFT spectrum
TODO: Check why LP Butterworth filter seems to introduce phase shift 
"""

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


BASE_SIGNAL_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../sounds/daniel_guitar_mono_trimmed.wav")
OUTPUT_AUDIO_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../sounds/output_sounds")

if __name__ == "__main__":
    sample_rate, data, num_channels = UF.wavread(BASE_SIGNAL_PATH)
    print(f"number of channels = {num_channels}")

    if num_channels > 1:
        raise ValueError("Base file should be mono!")

    num_samples = data.shape[0]
    length = num_samples / sample_rate
    print(f"length = {length}s")

    frequencies, mag_db = db_fft(data, sample_rate)
    print("num samples: ", num_samples, " frequencies: ", len(frequencies), ", mag_db: ", len(mag_db))

    time = np.linspace(0., length, data.shape[0])

    # Plot base signal and db mag spectrum
    plt.subplot(411)
    plt.plot(time, data)
    plt.legend()
    plt.title("Base signal")
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")
    plt.subplot(412)
    plt.plot(frequencies, mag_db)
    plt.title("Base signal spectrum")
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(mag_db)])

    # Low pass filter the base signal
    filtered_base_signal = lowpass.butter_lowpass_filter(data, 18000, sample_rate, order=12)
    filtered_base_frequencies, filtered_base_mag_db = db_fft(filtered_base_signal, sample_rate)

    UF.wavwrite(filtered_base_signal, sample_rate, os.path.join(OUTPUT_AUDIO_PATH, "filtered_base_signal.wav"))

    plt.subplot(413)
    plt.plot(time, filtered_base_signal)
    plt.legend()
    plt.title("Filtered signal")
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")
    plt.subplot(414)
    plt.plot(filtered_base_frequencies, filtered_base_mag_db)
    plt.title("Filtered signal spectrum")
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(filtered_base_mag_db)])

    plt.subplots_adjust(hspace=0.35)
    plt.show()

