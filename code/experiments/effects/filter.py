import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
from filters.lowpass import butter_lowpass
from scipy.signal import sosfilt, sosfreqz, sosfilt_zi
import utils_functions as UF
import numpy as np

'''
An attempt at a real time low pass filter

Warning: The result is very choppy sounding (needs more research)
'''

class LowpassFilter:

    def __init__(self, cutoff = 800, order = 5, sample_rate=44100):
        self.cutoff = cutoff
        self.order = order
        self.sample_rate = sample_rate
        self.filter = butter_lowpass(cutoff, sample_rate=sample_rate, order=order)
        self.zi = sosfilt_zi(self.filter)

    def __call__(self, val):
        f_val, self.zi = sosfilt(self.filter, np.atleast_1d(val), zi=self.zi)
        return f_val[0]        


if __name__ == "__main__":

    from synthesis.oscillator import SawtoothOscillator
    from synthesis.chain import Chain
    from sfx import DummyModifier

    osc_freq = 200
    sample_rate = 44100
    cutoff = 600
    order=1
    sos = butter_lowpass(cutoff, sample_rate, order=order)
    # w, h = sosfreqz(sos, fs=sample_rate, worN=8000)

    # import matplotlib.pyplot as plt

    # plt.subplot(211)
    # plt.plot(w, np.abs(h))
    # plt.plot(cutoff, 0.5*np.sqrt(2), 'ko')
    # plt.title('Lowpass filter frequency response')
    # plt.xlabel('Frequency [Hz]')
    # plt.ylabel('Amplitude')
    # plt.xlim(0, 0.5*sample_rate)
    # plt.grid(which='both', axis='both')
    # plt.axvline(cutoff, color='green')
    # plt.show()

    unfiltred_chain = Chain(SawtoothOscillator(osc_freq), DummyModifier())
    unfiltered_signal = unfiltred_chain.getNextBlockCallback(sample_rate)

    filtred_chain = Chain(SawtoothOscillator(osc_freq), LowpassFilter(cutoff, order))
    filtered_signal = filtred_chain.getNextBlockCallback(sample_rate)
    
    UF.plot_sigspectrum(unfiltered_signal, fslice=slice(0,800), sig_title="Signal: {} Hz".format(osc_freq), spec_title="Spectrum: {} Hz".format(osc_freq))
    UF.plot_sigspectrum(filtered_signal, fslice=slice(0,800), sig_title="Signal: {} Hz".format(osc_freq), spec_title="Spectrum: {} Hz".format(osc_freq))

    from realtime_player import stream_audio
    iter(filtred_chain)
    stream_audio(filtred_chain.getNextBlockCallback)
