from abc import ABC, abstractmethod
import math
import numpy as  np

class Oscillator(ABC):
    def __init__(self, freq, amp, sample_rate):
        self.freq = freq
        self.amp = amp
        self.sample_rate = sample_rate
        self._iter = iter(self)

    def __iter__(self):
        self._initialize_osc()
        return self

    def _initialize_osc(self):
        pass

    def getNextBlockCallback(self, num_samples):
        return [next(self._iter) for i in range(num_samples)]

    @abstractmethod
    def __next__(self):
        pass

class SineOscillator(Oscillator):

    def _initialize_osc(self):
        self._step = (2 * np.pi * self.freq) / self.sample_rate
        self._i = 0

    def __next__(self):
        val = np.sin(self._i)
        self._i += + self._step

        return val * self.amp

class SquareOscillator(Oscillator):
    def _initialize_osc(self):
        self._step = 1/self.sample_rate
        self._i = 0

    def __next__(self):
        val = 2 * (2 * math.floor(self.freq*self._i) - math.floor(2*self.freq*self._i)) + 1
        self._i += + self._step
        return val * self.amp

if __name__ == "__main__":
    sine_osc = SineOscillator(440, 1, 44100)
    sine_signal = sine_osc.getNextBlockCallback(1024)
    # print(sine_signal)

    sqr_osc = SquareOscillator(440, 1, 44100)
    sqr_signal = sqr_osc.getNextBlockCallback(1024)
    # print(sqr_signal)

    import sys, os
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    import utils_functions as UF
    import matplotlib.pyplot as plt

    times = np.linspace(0, len(sqr_signal), len(sqr_signal))
    UF.signal_plot(1,1,1, times, sqr_signal)
    plt.show()
