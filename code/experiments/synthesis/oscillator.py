from abc import ABC, abstractmethod
import math
import numpy as  np

class Oscillator(ABC):
    def __init__(self, freq=440, amp=1, phase=0, sample_rate=44100):

        # Fixed, unchanging initialised values
        self._freq = freq
        self._amp = amp
        self._phase = phase
        self.sample_rate = sample_rate

        # Properties that will be changed
        self._f = freq
        self._a = amp
        self._p = phase

        self._iter = iter(self)
    
    @property
    def initial_freq(self):
        return self._freq

    @property 
    def initial_amp(self):
        return self._amp

    @property 
    def initial_phase(self):
        return self._phase

    @property
    def freq(self):
        return self._f

    @freq.setter
    def freq(self, v):
        self._f = v
        self._post_freq_set()

    @property
    def amp(self):
        return self._a

    @amp.setter
    def amp(self, v):
        self._a = v
        self._post_amp_set()

    @property
    def phase(self):
        return self._p

    @phase.setter
    def phase(self, v):
        self._p = v
        self._post_phase_set()

    def getNextBlockCallback(self, num_samples):
        '''
        TODO: Consider removing this - since we're working with iterators
        the higher level structures (e.g. synth, waveadder etc) are iterators themselves 
        they could define this method but on the low level OSC stuff needs to return
        value by value for the approach to make sense
        '''
        return [next(self._iter) for i in range(num_samples)]

    def _post_freq_set(self):
        pass

    def _post_amp_set(self):
        pass

    def _post_phase_set(self):
        pass

    def __iter__(self):
        '''
        This ensures each time the iter is created the 
        values are reset to the initial values
        '''
        self.freq = self._freq
        self.amp = self._amp
        self.phase = self._phase
        self._initialize_osc()

        return self

    @abstractmethod
    def _initialize_osc(self):
        pass

    @abstractmethod
    def __next__(self):
        pass

class SineOscillator(Oscillator):
    def _post_freq_set(self):
        self._step = (2 * np.pi * self._f) / self.sample_rate

    def _post_phase_set(self):
        self._p = (self._p / 360) * 2 * np.pi

    def _initialize_osc(self):
        self._i = 0

    def __next__(self):
        val = np.sin(self._i + self._p)
        self._i += self._step
        return val * self._a

class SquareOscillator(Oscillator):
    def _initialize_osc(self):
        self._step = 1/self.sample_rate
        self._i = 0

    def __next__(self):
        val = 2 * (2 * math.floor(self._f*self._i) - math.floor(2*self._f*self._i)) + 1
        self._i += + self._step
        return val * self._a

if __name__ == "__main__":
    sine_osc = SineOscillator()
    sine_signal = sine_osc.getNextBlockCallback(1024)
    print(sine_signal)

    sqr_osc = SquareOscillator()
    sqr_signal = sqr_osc.getNextBlockCallback(1024)

    import sys, os
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    import utils_functions as UF
    import matplotlib.pyplot as plt

    times = np.linspace(0, len(sqr_signal), len(sqr_signal))
    UF.signal_plot(1,1,1, times, sine_signal)
    plt.show()
