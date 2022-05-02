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

class Transmitter:
    """
    A class for hiding a message signal inside a base signal. 

    This class will perform an operation to low pass the base signal with a cutoff bpf_lowcut (Above this range
    is where the message signal will be hidden) and perform a bandpass operation on the message signal.
    It performs frequency modulation on the message signal to put the message signal into the desired high frequency range
    (this will be outside of the frequency range of human hearing) and combine the signals into a final combined signal

    arguments:
        base_filepath: the full filepath to where the base signal exists on disc (mono wav file)
        message_filepath: the full filepath to where the message to be hidden exists on disc (mono wav file)
        lpf_cutoff: the low pass cutoff frequency to use for the low pass filtering operation of the base signal (Hz)
        bpf_lowcutoff: the lower cutoff frequency to use for the bandpass filtering operation of the message signal (Hz)
        bpf_highpass: the upper cutoff frequency to use for the bandpass filtering operation of the message signal (Hz)

        Note: The number of channels & sample rate of the base signal & the message signal should be the same
    """

    def __init__(self, base_filepath, message_filepath, lpf_cutoff = 18000.0, bpf_lowcutoff = 300.0, bpf_highcutoff = 3300.0):
        self.base_data, self.sample_rate, self.num_channels, self.base_num_samples = self._load_audio(base_filepath)
        self.message_data, message_sample_rate, num_channels, self.message_num_samples = self._load_audio(message_filepath)

        if message_sample_rate != self.sample_rate:
            raise ValueError("Base and message sample rates differ, they must be the same, base=", self.sample_rate, " message=", message_sample_rate)

        if num_channels != self.num_channels:
            raise ValueError("Base and message channels differ, they must be the same, base=", self.num_channels, " message=", num_channels)
        
        self.lpf_cuttoff = lpf_cutoff
        self.bpf_lowcutoff = bpf_lowcutoff
        self.bpf_highcutoff = bpf_highcutoff

        self.filtered_base_signal = []
        self.filtered_message_signal = []
        self.combined_signal = []

        self._perform()

    def set_base_lowpass_cutoff(freq):
        pass

    def set_message_bandpass_cutoff(low_freq, high_freq):
        pass

    def plot(self):
         # Plot base signal and db mag spectrum
        base_length = self.base_num_samples / self.sample_rate
        base_time = np.linspace(0., base_length, self.base_num_samples)
        base_frequencies, base_mag_db = self._db_fft(self.base_data)
        plt.subplot(421)
        plt.plot(base_time, self.base_data)
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

        filtered_base_frequencies, filtered_base_mag_db = self._db_fft(self.filtered_base_signal)
        plt.subplot(425)
        plt.plot(base_time, self.filtered_base_signal)
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

        message_length = self.message_num_samples / self.sample_rate
        message_time = np.linspace(0, message_length, self.message_num_samples)
        message_frequencies, message_mag_db = self._db_fft(self.message_data)
        plt.subplot(422)
        plt.plot(message_time, self.message_data)
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

        filtered_message_frequencies, filtered_message_mag_db = self._db_fft(self.filtered_message_signal)
        plt.subplot(426)
        plt.plot(message_time, self.filtered_message_signal)
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

        plt.tight_layout()
        plt.show()


    def write(self, output_directory, write_intermediate=False):
        """
        Writes the combined audio to disk in the specified output_directory

        Optionally writes the intermediate filtered base & filtered message signals to disk in the same output_directory
        (this is mainly useful for debugging purposes)
        """

        if write_intermediate:
            UF.wavwrite(self.filtered_base_signal, self.sample_rate, os.path.join(output_directory, "filtered_base_signal.wav"))
            UF.wavwrite(self.filtered_message_signal, self.sample_rate, os.path.join(output_directory, "filtered_message_signal.wav"))


    def _load_audio(self, file_path):
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


    def _db_fft(self, in_data):
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
        frequencies = rfftfreq(data_length, d=1/self.sample_rate)
        magnitude_spectrum = np.abs(fft_values) * 2 / np.sum(weighting)
        magnitude_dbs = 10*np.log10(magnitude_spectrum)

        return frequencies, magnitude_dbs


    def _perform(self):
        """
        Performs the message hiding operation. 
        The basic steps are:
            1. low pass the data contained in base_data
            2. band pass the data contained in message_data
            3. perform frequency modulation on message_data to put it into the high frequency domain outside of human hearing
            4. combine the signals into one containing both the base_data and the hidden message

        """
        
         # Low pass filter the base signal
        self.filtered_base_signal = lowpass.butter_lowpass_filter(self.base_data, 18000, self.sample_rate, order=12)
       
         # band pass filter the message signal
        self.filtered_message_signal = bandpass.butter_bandpass_filter(self.message_data, 300, 3300, self.sample_rate, order=6)
        
        # perform the frequency modulation


if __name__ == "__main__":
    current_file_path = os.path.dirname(os.path.realpath(__file__))
    base_signal_path = os.path.join(current_file_path, "../sounds/daniel_guitar_mono_trimmed.wav")
    message_signal_path = os.path.join(current_file_path, "../sounds/secret_message.wav")
    output_audio_path = os.path.join(current_file_path, "../sounds/output_sounds")

    transmitter = Transmitter(base_signal_path, message_signal_path)
    transmitter.write(output_audio_path, write_intermediate=True)
    transmitter.plot()
