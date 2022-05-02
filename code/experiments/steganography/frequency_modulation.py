import matplotlib.pyplot as plt
import numpy as np
from scipy.fft import fft, fftfreq, rfft, rfftfreq
import os, sys 
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF
from filters import lowpass, bandpass

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

def load_audio(file_path):
    """
    Load an audio wav file located at file_path
    Note: should be a mono file

    returns:
        data - floating point array of audio samples with range [-1,1]
        sample_rate - the sample rate of the audio data
        num_channels - the number of channels of audio data
        num_samples - the number of samples in the data

    """
    sample_rate, data, num_channels = UF.wavread(file_path)
    print(f"number of channels = {num_channels}")

    if num_channels > 1:
        raise ValueError("Base file should be mono!")

    num_samples = data.shape[0]

    return data, sample_rate, num_channels, num_samples



CURRENT_FILE_PATH = os.path.dirname(os.path.realpath(__file__))
BASE_SIGNAL_PATH = os.path.join(CURRENT_FILE_PATH, "../sounds/daniel_guitar_mono_trimmed.wav")
MESSAGE_SIGNAL_PATH = os.path.join(CURRENT_FILE_PATH, "../sounds/secret_message.wav")
OUTPUT_AUDIO_PATH = os.path.join(CURRENT_FILE_PATH, "../sounds/output_sounds")

if __name__ == "__main__":
    # Load the base signal, plot and apply filtering operations
    base_signal, base_sample_rate, base_num_channels, base_num_samples = load_audio(BASE_SIGNAL_PATH)
    base_length = base_num_samples / base_sample_rate
    print(f"Base length = {base_length}s")

    base_frequencies, base_mag_db = db_fft(base_signal, base_sample_rate)
    print("Base: num samples: ", base_num_samples, " frequencies: ", len(base_frequencies), ", mag_db: ", len(base_mag_db))

    base_time = np.linspace(0., base_length, base_num_samples)

    # Plot base signal and db mag spectrum
    plt.subplot(421)
    plt.plot(base_time, base_signal)
    plt.legend()
    plt.title("Base signal")
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")
    plt.subplot(423)
    plt.plot(base_frequencies, base_mag_db)
    plt.title("Base signal spectrum")
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(base_mag_db)])

    # Low pass filter the base signal
    filtered_base_signal = lowpass.butter_lowpass_filter(base_signal, 18000, base_sample_rate, order=12)
    filtered_base_frequencies, filtered_base_mag_db = db_fft(filtered_base_signal, base_sample_rate)
    UF.wavwrite(filtered_base_signal, base_sample_rate, os.path.join(OUTPUT_AUDIO_PATH, "filtered_base_signal.wav"))

    plt.subplot(425)
    plt.plot(base_time, filtered_base_signal)
    plt.legend()
    plt.title("Filtered base signal")
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")
    plt.subplot(427)
    plt.plot(filtered_base_frequencies, filtered_base_mag_db)
    plt.title("Filtered base spectrum")
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(filtered_base_mag_db)])

    # Load the secret message signal, plot and apply filtering operations
    message_signal, message_sample_rate, message_num_channels, message_num_samples = load_audio(MESSAGE_SIGNAL_PATH)
    message_length = message_num_samples / message_sample_rate
    print(f"Message length = {message_length}s")

    message_frequencies, message_mag_db = db_fft(message_signal, message_sample_rate)
    print("Message: num samples: ", message_num_samples, " frequencies: ", len(message_frequencies), ", mag_db: ", len(message_mag_db))

    message_time = np.linspace(0.0, message_length, message_num_samples)
    plt.subplot(422)
    plt.plot(message_time, message_signal)
    plt.legend()
    plt.title("Message signal")
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")
    plt.subplot(424)
    plt.plot(message_frequencies, message_mag_db)
    plt.title("Message signal spectrum")
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(message_mag_db)])


    # band pass filter the message signal
    filtered_message_signal = bandpass.butter_bandpass_filter(message_signal, 300, 3300, message_sample_rate, order=6)
    filtered_message_frequencies, filtered_message_mag_db = db_fft(filtered_message_signal, message_sample_rate)
    UF.wavwrite(filtered_message_signal, message_sample_rate, os.path.join(OUTPUT_AUDIO_PATH, "filtered_message_signal.wav"))

    plt.subplot(426)
    plt.plot(message_time, filtered_message_signal)
    plt.legend()
    plt.title("Filtered message signal")
    plt.xlabel("Time [s]")
    plt.ylabel("Amplitude")
    plt.subplot(428)
    plt.plot(filtered_message_frequencies, filtered_message_mag_db)
    plt.title("Filtered Message spectrum")
    plt.xlabel("Frequency [Hz]")
    plt.ylabel("Amplitude (dB)")
    plt.ylim([-100, np.amax(filtered_message_mag_db)])

    plt.subplots_adjust(vspace=0.35)
    plt.show()

