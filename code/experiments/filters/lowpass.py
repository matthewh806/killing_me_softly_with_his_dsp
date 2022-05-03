import numpy as np
from scipy.signal import butter, sosfilt, sosfreqz

"""
TODO: 
    - There seems to be a slight phase shift indroduced by the lowpass filtering operation
      To most clearly see this do something like:
        input_signal = UF.generateSineSignal(10, T, sample_rate)
        output_signal = butter_lowpass_filter(input_signal, cutoff, sample_rate, order=6)
        with a sample_rate = 400, cutoff = 80 (so no filtering should occur)

        plot the results together and observe the slight offset in phase
"""

def butter_lowpass(cutoff, sample_rate, order=5):
    """
    Designs a digital lowpass butterworth filter of order N with a given cutoff frequency
    
    Returns:
        b, a : ndarray, ndarray
        Numerator (b) and denominator (a) polynomials of the IIR filter
    """

    return butter(order, cutoff, fs=sample_rate, btype='low', analog=False, output='sos')


def butter_lowpass_filter(data, cutoff, sample_rate, order):
    """
    Performs a low pass filtering operation on input data by designing a
    lowpass butterworth filter of a specified order and sample_rate

    Returns:
        y: The output of the lowpass digital filter
    """

    sos = butter_lowpass(cutoff, sample_rate, order)
    return sosfilt(sos, data)


if __name__ == "__main__":
    import matplotlib.pyplot as plt

    cutoff = 18000.0
    sample_rate = 44100.0
    order = 48

    # Design a filter to print the frequency response
    sos = butter_lowpass(cutoff, sample_rate, order=order)
    w, h = sosfreqz(sos, fs=sample_rate, worN=8000)

    plt.subplot(211)
    plt.plot(w, np.abs(h))
    plt.plot(cutoff, 0.5*np.sqrt(2), 'ko')
    plt.title('Lowpass filter frequency response')
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Amplitude')
    plt.xlim(0, 0.5*sample_rate)
    plt.grid(which='both', axis='both')
    plt.axvline(cutoff, color='green')

    import os, sys
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    import utils_functions as UF
    # Demonstrate the use of the filter
    T = 1.0 # 1 second of data
    t = np.linspace(0, T, int(T * sample_rate), endpoint=False)
    input_signal = UF.generateSineSignal(10, T, sample_rate) + UF.generateSineSignal(150, T, sample_rate)
    filtered_signal = butter_lowpass_filter(input_signal, cutoff, sample_rate, order=order)
    
    plt.subplot(212)
    plt.plot(t, input_signal, 'b-', label='input')
    plt.plot(t, filtered_signal, 'g-', linewidth=3, label='filtered')
    plt.xlabel('Time [s]')
    plt.ylabel("amplitude")
    plt.grid()
    plt.legend()

    plt.subplots_adjust(hspace=0.35)
    plt.show()