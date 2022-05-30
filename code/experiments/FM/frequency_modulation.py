import numpy as np

def frequency_modulate_signal(modulation_signal, carrier_frequency, modulation_index, sample_rate):
    signal_length = len(modulation_signal) / sample_rate
    samples = np.arange(signal_length * float(sample_rate)) / float(sample_rate)
    return np.sin(2.0 * np.pi * carrier_frequency * samples + modulation_index * modulation_signal)

if __name__ == "__main__":
    import sys, os
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    from experiments.utils_functions import generateSineSignal

    modulation_frequency = 200.0
    carrier_frequency = 400.0
    sample_rate = 44100.0

    modulation_signal = generateSineSignal(modulation_frequency, 1.0, sample_rate)
    carrier_signal = generateSineSignal(carrier_frequency, 1.0, sample_rate)
    frequency_modulated_signal = frequency_modulate_signal(modulation_signal, carrier_frequency, modulation_index=1, sample_rate=sample_rate)

    samples_to_plot = 1024
    import matplotlib.pyplot as plt
    plt.subplot(211)
    plt.plot(modulation_signal[:samples_to_plot], label="Modulation Signal")
    plt.plot(carrier_signal[:samples_to_plot], label="Carrier Signal")
    plt.plot(frequency_modulated_signal[:samples_to_plot], label="Frequency Modulated Signal")
    plt.legend()
    plt.title('Signals')
    plt.xlabel('samples')
    plt.ylabel('Amplitude')
    plt.grid(which='both', axis='both')
    plt.subplot(212)
    plt.specgram(frequency_modulated_signal, 2048, Fs=sample_rate)
    plt.xlabel("Time [s]")
    plt.ylabel("Frequency [Hz]")
    plt.tight_layout()
    plt.show()