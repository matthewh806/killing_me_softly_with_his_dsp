import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
from synthesis.oscillator import SineOscillator, SquareOscillator
from synthesis.modulated_oscillator import ModulatedOscillator, amp_mod
from synthesis.envelope import ADSREnvelope
import utils_functions as UF
import numpy as np

import matplotlib.pyplot as plt

def identity_transfer_function(x):
    '''
    Return the wave untouched
    '''
    return x

class WaveShaper():
    '''
    Distort the shape of an incoming waveform (oscillator) by feeding it
    into a transfer function

    The transfer function should be of the form e.g.

    def transfer_function(x):
        return x**2

    i.e. take in a value x and transform it into some y(x)
    '''

    def __init__(self, oscillator, transfer_function=identity_transfer_function):
        self.oscillator = oscillator
        self.transfer_function = transfer_function

    def getNextBlockCallback(self, num_samples):
        return [next(self) for i in range(num_samples)]

    def __iter__(self):
        iter(self.oscillator)
        return self

    def __next__(self):
        val = next(self.oscillator)

        return self.transfer_function(val)

def plot_waveshaper_signal(osc, transfer_function=identity_transfer_function, title="Waveshaped Signal"):
        gen = WaveShaper(osc, transfer_function)
        iter(gen)

        signal = gen.getNextBlockCallback(44100)
        fig = plt.figure(figsize=(25, 6.25))
        UF.signal_plot(1, 1, 1, signal, title=title)
        plt.show()

        return signal

if __name__ == "__main__":

    def clipping_transfer_function(x, min_out, max_out):
        return max(min_out, min(x, max_out))

    plot_waveshaper_signal(SineOscillator(20), title="Identity fn")
    plot_waveshaper_signal(SineOscillator(20), lambda x: x*x, title="Square fn")
    plot_waveshaper_signal(SineOscillator(20), lambda x: x/2, title="Half fn")
    plot_waveshaper_signal(SineOscillator(20), lambda x: clipping_transfer_function(x, -0.7, 0.7), "Cliping fn")

    from realtime_player import stream_audio
    clipped_shaper = WaveShaper(SineOscillator(200), lambda x: clipping_transfer_function(x, -0.9, 0.9))
    stream_audio(clipped_shaper.getNextBlockCallback)
