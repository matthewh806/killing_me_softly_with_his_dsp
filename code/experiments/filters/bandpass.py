import numpy as np
from scipy.signal import butter, sosfilt, sosfreqz

"""
TODO: 
    - The output shape is not symmetric 
"""

def butter_bandpass(lowcut, highcut, sample_rate, order=5):
    """
    Designs a digital bandpass butterworth filter of order N with a given lowcut & 
    highcut frequnecy
    """

    return butter(order, [lowcut, highcut], fs=sample_rate, btype='bandpass', analog=False, output='sos')


def butter_bandpass_filter(data, lowcut, highcut, sample_rate, order):
    """
    Performs a bandpass filtering operation on input data by designing a
    bandpass butterworth filter of a specified order and sample_rate and with
    cutoff points specified by lowcut and highcut

    Returns:
        y: The output of the lowpass digital filter
    """

    sos = butter_bandpass(lowcut, highcut, sample_rate, order)
    return sosfilt(sos, data)


if __name__ == "__main__":
    import matplotlib.pyplot as plt

    lowcut = 300    
    highcut = 3300
    sample_rate = 44100.0
    order = 48

    # Design a filter to print the frequency response
    sos = butter_bandpass(lowcut, highcut, sample_rate, order=order)
    w, h = sosfreqz(sos, fs=sample_rate, worN=8000)

    plt.subplot(211)
    plt.plot(w, np.abs(h))
    plt.plot(lowcut, 0.5*np.sqrt(2), 'ko')
    plt.plot(highcut, 0.5*np.sqrt(2), 'ko')
    plt.title('Lowpass filter frequency response')
    plt.xlabel('Frequency [Hz]')
    plt.ylabel('Amplitude')
    plt.xlim(0, 0.5*sample_rate)
    plt.grid(which='both', axis='both')
    plt.axvline(lowcut, color='green')
    plt.axvline(highcut, color='green')

    import sys, os
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    import utils_functions as UF
    # Demonstrate the use of the filter
    T = 1.0 # 1 second of data
    t = np.linspace(0, T, int(T * sample_rate), endpoint=False)
    input_signal = UF.generateSineSignal(10, T, sample_rate) + UF.generateSineSignal(130, T, sample_rate) + UF.generateSineSignal(190, T, sample_rate)
    filtered_signal = butter_bandpass_filter(input_signal, lowcut, highcut, sample_rate, order=order)
    
    plt.subplot(212)
    plt.plot(t, input_signal, 'b-', label='input')
    plt.plot(t, filtered_signal, 'g-', linewidth=3, label='filtered')
    plt.xlabel('Time [s]')
    plt.ylabel("amplitude")
    plt.grid()
    plt.legend()

    plt.subplots_adjust(hspace=0.35)
    plt.show()

    plt.show()