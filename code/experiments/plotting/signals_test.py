import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF
import matplotlib.pyplot as plt
import numpy as np
from scipy.io.wavfile import write
from scipy.fft import fft, fftfreq

'''
Plots of basic functions. 

Triangle waves display only odd harmonics in the spectra, roll off at a rate 1/(harmonic_no)^2
Square waves display only odd harmonics in the spectra, roll off at a rate 1/harmonic_no
Sawtooth waves display even and harmonics in the spectra, roll off at a rate 1/harmonic_no
'''

def printStrongestHarmonics(freqs, fft_data, n):
    sorted_indices = (-fft_data).argsort()[:n]
    for hn, amp in zip(freqs[sorted_indices], fft_data[sorted_indices]):
        print("{}: {}".format(hn, amp))

if __name__ == "__main__":
    freq = 900 # Hz
    sample_rate=44100.0
    signal_length = 2.0 # seconds
    period = 1/freq

    # Generate the signals
    num_samples = int(signal_length * sample_rate)
    tri_signal = UF.generateTriangleSignal(freq, signal_length, sample_rate)
    sqr_signal = UF.generateSquareSignal(freq,signal_length, sample_rate)
    saw_signal = UF.generateSawtoothSignal(freq, signal_length, sample_rate)
    times = np.linspace(0, signal_length, num_samples)

    # Generate the spectra
    tri_ffts = fft(tri_signal)
    tri_amps = 1.0/num_samples * np.abs(tri_ffts[:num_samples//2])
    tri_min_amp = np.amin(tri_amps)

    sqr_ffts = fft(sqr_signal)
    sqr_amps =  2.0/num_samples * np.abs(sqr_ffts[:num_samples//2])
    sqr_min_amp = np.amin(sqr_amps)

    saw_ffts = fft(saw_signal)
    print(saw_ffts)
    saw_amps = 2.0/num_samples * np.abs(saw_ffts[:num_samples//2])
    saw_min_amp = np.amin(saw_amps)

    print(saw_amps)
    print("finite: ", np.any(np.isfinite(tri_amps)))

    freqs = fftfreq(num_samples, 1/sample_rate)[:num_samples//2]

    # Print the top highest harmonic dB values
    print("Triangle Harmonics: ")
    printStrongestHarmonics(freqs, tri_amps, 10)

    print("Square Harmonics: ")
    printStrongestHarmonics(freqs, sqr_amps, 10)

    print("Saw Harmonics: ")
    printStrongestHarmonics(freqs, saw_amps, 10)

    # Write the signal wav to disk
    current_file_directory = os.path.dirname(os.path.realpath(__file__))
    output_sounds_directory = os.path.join(current_file_directory, "../sounds/output_sounds")

    write(os.path.join(output_sounds_directory, "triangle_{0}Hz.wav".format(freq)), int(sample_rate), tri_signal)
    write(os.path.join(output_sounds_directory, "square_{0}Hz.wav".format(freq)), int(sample_rate), sqr_signal)
    write(os.path.join(output_sounds_directory, "saw_{0}Hz.wav".format(freq)), int(sample_rate), saw_signal)

    # Plot the signals
    UF.signal_plot(2,3,1, times, tri_signal, "Tri Signal")
    UF.spectrum_plot(2,3,4, freqs, tri_amps, "Tri Spectrum")
    plt.ylim(ymin=tri_min_amp)
    UF.signal_plot(2,3,2, times, sqr_signal, "Sqr Signal")
    UF.spectrum_plot(2,3,5, freqs, sqr_amps, "Sqr Spectrum")
    plt.ylim(ymin=sqr_min_amp)
    UF.signal_plot(2,3,3, times, saw_signal, "Saw Signal")
    UF.spectrum_plot(2,3,6, freqs, saw_amps, "Saw Spectrum")
    plt.ylim(ymin=sqr_min_amp)
    plt.show()
