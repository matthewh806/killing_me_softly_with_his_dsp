from cmath import pi
import matplotlib.pyplot as plt
import numpy as np
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
from spectral.fft import db_fft
from filters.bandpass import butter_bandpass_filter
from filters.lowpass import butter_lowpass_filter 
from FM.frequency_modulation import frequency_modulate_signal
import utils_functions as UF
import logging

"""
Note: Only handle mono files for the time being

TODO: Check the filtering operation when cutoff ~300Hz causes inf values and a messed up FFT spectrum
TODO: Check why LP Butterworth filter seems to introduce phase shift 
"""

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
    logging.info("Loading file: %s, number of channels: %i, sample rate: %f", file_path, num_channels, sample_rate)

    if num_channels > 1:
        raise ValueError("Base file should be mono!")

    num_samples = data.shape[0]
    return data, sample_rate, num_channels, num_samples



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
        order: The order of the filter to create (note: same for both lpf & bpf) (int, dimensionless)

        Note: The number of channels & sample rate of the base signal & the message signal should be the same
    """

    def __init__(self, base_filepath, message_filepath, lpf_cutoff = 18000.0, bpf_lowcutoff = 300.0, bpf_highcutoff = 3300.0, order = 6):
        self.base_data, self.sample_rate, self.num_channels, self.base_num_samples = load_audio(base_filepath)
        self.message_data, message_sample_rate, num_channels, self.message_num_samples = load_audio(message_filepath)

        if message_sample_rate != self.sample_rate:
            raise ValueError("Base and message sample rates differ, they must be the same, base=", self.sample_rate, " message=", message_sample_rate)

        if num_channels != self.num_channels:
            raise ValueError("Base and message channels differ, they must be the same, base=", self.num_channels, " message=", num_channels)
        
        self.lpf_cuttoff = lpf_cutoff
        self.bpf_lowcutoff = bpf_lowcutoff
        self.bpf_highcutoff = bpf_highcutoff
        self.filter_order = order

        self.filtered_base_signal = []
        self.filtered_message_signal = []
        self.frequency_shifted_message_signal = []
        self.combined_signal = []

    def set_base_lowpass_cutoff(freq):
        pass


    def set_message_bandpass_cutoff(low_freq, high_freq):
        pass


    def save_plots(self, output_directory):
        self._plot(output_directory)


    def write(self, output_path, write_intermediate=False):
        """
        Writes the combined audio to disk in the specified output_path

        Optionally writes the intermediate filtered base & filtered message signals to disk in the same directory as output_path
        (this is mainly useful for debugging purposes)
        """

        UF.wavwrite(self.combined_signal, self.sample_rate, output_path)

        if write_intermediate:
            output_directory = os.path.dirname(output_path)

            UF.wavwrite(self.filtered_base_signal, self.sample_rate, os.path.join(output_directory, "filtered_base_signal.wav"))
            UF.wavwrite(self.filtered_message_signal, self.sample_rate, os.path.join(output_directory, "filtered_message_signal.wav"))
            UF.wavwrite(self.frequency_shifted_message_signal, self.sample_rate, os.path.join(output_directory, "shifted_message_signal.wav"))


    def perform(self, carrier_frequency = 20000.0, modulation_index = 1.0):
        """
        Performs the message hiding operation. 
        The basic steps are:
            1. low pass the data contained in base_data
            2. band pass the data contained in message_data
            3. perform frequency modulation on message_data to put it into the high frequency domain outside of human hearing
            4. combine the signals into one containing both the base_data and the hidden message

        """
        
        logging.info("Performing frequency modulation: low pass frequency: %f, band pass frequencies: (%f, %f), order: %f, carrier frequency: %f, modulation index: %i",
                        self.lpf_cuttoff, self.bpf_lowcutoff, self.bpf_highcutoff, self.filter_order, carrier_frequency, modulation_index)

         # Low pass filter the base signal
        self.filtered_base_signal = butter_lowpass_filter(self.base_data, self.lpf_cuttoff, self.sample_rate, order=self.filter_order)
       
         # band pass filter the message signal
        self.filtered_message_signal = butter_bandpass_filter(self.message_data, self.bpf_lowcutoff, self.bpf_highcutoff, self.sample_rate, order=self.filter_order)
        
        # perform the frequency modulation
        self.frequency_shifted_message_signal = frequency_modulate_signal(self.filtered_message_signal, carrier_frequency, modulation_index, self.sample_rate)

        # Combine the filtered base signal with the filtered and frequency modulated message signal
        base_num_samples = len(self.filtered_base_signal)
        message_num_samples = len(self.frequency_shifted_message_signal)
        if message_num_samples < base_num_samples:
            pad_size = base_num_samples - message_num_samples
            logging.debug("Padding message signal with: %i zeros", pad_size)
            self.frequency_shifted_message_signal = np.pad(self.frequency_shifted_message_signal, (0, pad_size))
        elif message_num_samples > base_num_samples:
            # TODO: Warn and truncate
            pass

        self.combined_signal = self.filtered_base_signal + self.frequency_shifted_message_signal

    def _plot(self, output_directory):
         # Plot base signal and db mag spectrum
        base_length = self.base_num_samples / self.sample_rate
        base_time = np.linspace(0., base_length, self.base_num_samples)
        base_frequencies, base_mag_db = db_fft(self.base_data, self.sample_rate)

        message_length = self.message_num_samples / self.sample_rate
        message_time = np.linspace(0, message_length, self.message_num_samples)
        message_frequencies, message_mag_db = db_fft(self.message_data, self.sample_rate)

        UF.signal_plot(3,2,1, base_time, self.base_data, title = "Base Signal")
        UF.spectrum_plot(3,2,3, base_frequencies, base_mag_db, title = "Base Spectrum")
        UF.specgram_plot(3,2,5, self.base_data, self.sample_rate, title = "Base Spectrogram")
        UF.signal_plot(3,2,2, message_time, self.message_data, title = "Message Signal")
        UF.spectrum_plot(3,2,4, message_frequencies, message_mag_db, title="Message Spectrum")
        UF.specgram_plot(3,2,6, self.message_data, self.sample_rate, title="Message Spectrogram")
        plt.tight_layout()
        plt.savefig(os.path.join(output_directory, "input_signal_plots.png"))
        plt.close()

        filtered_base_frequencies, filtered_base_mag_db = db_fft(self.filtered_base_signal, self.sample_rate)
        shifted_message_frequncies, shifted_message_mag_db = db_fft(self.frequency_shifted_message_signal, self.sample_rate)
        shifted_message_length = len(self.frequency_shifted_message_signal) / self.sample_rate
        shifted_message_time = np.linspace(0, shifted_message_length, len(self.frequency_shifted_message_signal))
        UF.signal_plot(3,2,1, base_time, self.filtered_base_signal, title = "Filtered Base Signal")
        UF.spectrum_plot(3,2,3, filtered_base_frequencies, filtered_base_mag_db, title = "Filtered Base Spectrum")
        UF.specgram_plot(3,2,5, self.filtered_base_signal, self.sample_rate, title="Filtered Base Spectrogram")
        UF.signal_plot(3,2,2, shifted_message_time, self.frequency_shifted_message_signal, title = "Shifted Message Signal")
        UF.spectrum_plot(3,2,4, shifted_message_frequncies, shifted_message_mag_db, title = "Shifted Message Spectrum")
        UF.specgram_plot(3,2,6, self.frequency_shifted_message_signal, self.sample_rate, title="Filtered Message Spectrogram")
        plt.tight_layout()
        plt.savefig(os.path.join(output_directory, "filtered_and_shifted_plots.png"))
        plt.close()

        combined_frequencies, combined_mag_db = db_fft(self.combined_signal, self.sample_rate)
        UF.signal_plot(3,1,1, base_time, self.combined_signal, title = "Combined Signal")
        UF.spectrum_plot(3,1,2, combined_frequencies, combined_mag_db, title = "Combined Signal Spectrum")
        UF.specgram_plot(3,1,3, self.combined_signal, self.sample_rate, title = "Combined Signal Spectrogram")
        plt.tight_layout()
        plt.savefig(os.path.join(output_directory, "steganography_plot.png"))
        plt.close()


class Receiver:
    """
    A class for recovering an audio message hidden inside another

    This class will perform a bandpass filtering operation on the input signal, centered around the high frequency shifted range of
    the hidden signal, it will then demodulate the remaining signal back into the audio range using a carrier signal of 
    frequency equal to the midpoint of the original message signal in the human frequency range. This signal will then be lowpass filtered
    to remove any noise and unwanted high frequency content

    arguments:
        combined_signal_path: the full filepath to where the combined signal exists on disc (mono wav file)
        bpf_lowcutoff: the lower cutoff frequency to use for the bandpass filtering operation of the message signal (Hz)
        bpf_highpass: the upper cutoff frequency to use for the bandpass filtering operation of the message signal (Hz)
        order: The order of the filter to create (note: same for both lpf & bpf) (int, dimensionless)
    
        NOTE: The bpf cutoff frequencies supplied based on where the signal has been shifted into the higher frequency domain
              For example - if the original message signal used in the Transmitter was bandpassed at (300, 3.3k) Hz before being
              frequency modulated to 20 kHz the bandpass filter cutoff points for the reciever should then take this shift into 
              account. i.e. cutoff = (18000, 21800)
    """

    def __init__(self, combined_signal_path, bpf_lowcutoff = 300.0, bpf_highcutoff = 3300.0, order = 6):
        self.combined_data, self.sample_rate, self.num_channels, self.combined_num_samples = load_audio(combined_signal_path)
        
        self.bpf_lowcutoff = bpf_lowcutoff
        self.bpf_highcutoff = bpf_highcutoff
        self.filter_order = order

        self.recovered_message_signal = []


    def save_plots(self, output_directory):
        # Plot base signal and db mag spectrum
        self._plot(output_directory)
        

    def write(self, output_path):
        UF.wavwrite(self.recovered_message_signal, self.sample_rate, output_path)


    def perform(self, carrier_frequency = 1500.0, modulation_index = 1.0):
        """

        Performs the operation of retrieving the hidden message from the combined source
        The basic steps are:
            - Remove the base signal by bandpassing above and below the hidden message
            - Modulate the message back down to the audio range using a cosine carrier (what freq?)
            - Lowpass the demodulated signal to remove additive noise and higher frequencies
            - The result should be an approximation of the original message

        """

        logging.info("Performing frequency demodulation: band pass frequencies: (%f, %f), order: %f, carrier frequency: %f, modulation index: %i",
                        self.bpf_lowcutoff, self.bpf_highcutoff, self.filter_order, carrier_frequency, modulation_index)
        
        bandpassed_signal = butter_bandpass_filter(self.combined_data, self.bpf_lowcutoff, self.bpf_highcutoff, self.sample_rate, order=self.filter_order)

        # demodulate
        # this is based on the Transmitter having used an original bpf filter (300.0, 1500.0). 
        self.recovered_message_signal = frequency_modulate_signal(bandpassed_signal, carrier_frequency, modulation_index, self.sample_rate)
        self.recovered_message_signal = butter_lowpass_filter(self.recovered_message_signal, 3300.0, self.sample_rate, order=self.filter_order)

    def _plot(self, output_directory):
        plt.figure(figsize=(10,10))
        combined_length = self.combined_num_samples / self.sample_rate
        combined_time = np.linspace(0., combined_length, self.combined_num_samples)
        combined_frequencies, combined_mag_db = db_fft(self.combined_data, self.sample_rate)

        UF.signal_plot(3,2,1, combined_time, self.combined_data, title = "Combined Signal")
        UF.spectrum_plot(3,2,3, combined_frequencies, combined_mag_db, title = "Combined Spectrum")
        UF.specgram_plot(3,2,5, self.combined_data, self.sample_rate, title = "Combined Spectrogram")

        recovered_frequencies, recovered_mag_db = db_fft(self.recovered_message_signal, self.sample_rate)
        UF.signal_plot(3,2,2, combined_time, self.recovered_message_signal, title = "Recovered Secret Signal")
        UF.spectrum_plot(3,2,4, recovered_frequencies, recovered_mag_db, title = "Recovered Secret Spectrum")
        UF.specgram_plot(3,2,6, self.recovered_message_signal, self.sample_rate, title = "Combined Spectrogram")

        plt.tight_layout()
        plt.savefig(os.path.join(output_directory, "combined_and_recovered_signals.png"))
        plt.close()


if __name__ == "__main__":
    current_file_path = os.path.dirname(os.path.realpath(__file__))
    base_signal_path = os.path.join(current_file_path, "../sounds/daniel_guitar_mono_trimmed.wav")
    message_signal_path = os.path.join(current_file_path, "../sounds/secret_message.wav")
    output_audio_path = os.path.join(current_file_path, "../sounds/output_sounds")
    output_plot_path = os.path.join(current_file_path, "../plots")

    # first hide the secret message inside another audio file
    transmitter = Transmitter(base_signal_path, message_signal_path, lpf_cutoff=14000.0, order=96)
    transmitter.perform(modulation_index=1)
    combined_signal_path = os.path.join(output_audio_path, "combined_signal.wav")
    transmitter.write(combined_signal_path, write_intermediate=True)
    transmitter.save_plots(output_plot_path)

    # then recover the message from that combined audio file
    receiver = Receiver(combined_signal_path, order=48, bpf_lowcutoff=18500.0, bpf_highcutoff=21500.0)
    receiver.perform(modulation_index=1, carrier_frequency = 20000.0)
    receiver.write(os.path.join(output_audio_path, "recovered_secret_signal.wav"))
    receiver.save_plots(output_plot_path)
