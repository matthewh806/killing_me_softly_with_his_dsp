from abc import ABC, abstractmethod

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

if __name__ == "__main__":
    osc = SineOscillator(440, 1, 44100)
    signal = osc.getNextBlockCallback(1024)
    print(signal)
