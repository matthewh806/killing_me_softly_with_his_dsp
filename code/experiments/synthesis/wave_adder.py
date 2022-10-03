from oscillator import SineOscillator, SquareOscillator

class  WaveAdder:
    def __init__(self, *oscillators):
        self.oscillators = oscillators
        self.n = len(oscillators)

    def __iter__(self):
        [iter(osc) for osc in self.oscillators]
        return self

    def __next__(self):
        return sum(next(osc) for osc in self.oscillators) / self.n

    def getNextBlockCallback(self, num_samples):
        return [next(self) for i in range(num_samples)]

if __name__ == "__main__":
    sine_osc = SineOscillator(440)
    sin_osc_2 = SineOscillator(220)
    sqr_osc = SquareOscillator(560)

    waveAdder = WaveAdder(sine_osc, sin_osc_2, sqr_osc); iter(waveAdder)
    waveSumSignal = waveAdder.getNextBlockCallback(1024)

    import sys, os
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    import utils_functions as UF
    import matplotlib.pyplot as plt
    import numpy as np

    times = np.linspace(0, len(waveSumSignal), len(waveSumSignal))
    UF.signal_plot(1,1,1, times, waveSumSignal)
    plt.show()

    from realtime_player import stream_audio
    iter(waveAdder)
    stream_audio(waveAdder.getNextBlockCallback)
    
