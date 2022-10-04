import math
import numpy as np

'''
The Effects classes are just callables which transform a given input x in some way
and return this modified value
'''

class DummyModifier:

    def __call__(self, val):
        return val

class Bitcrusher:

    def __init__(self, bit_depth=16):
        self.bit_depth = bit_depth
        self._ql = self._get_quantisation_level()

    def __call__(self, val):
        return self._ql * int(val / self._ql)

    def _get_quantisation_level(self):
        return 2.0 / (pow(2.0, self.bit_depth) - 1.0)

def identity_transfer_function(x):
    '''
    Return the wave untouched
    '''
    return x

def clipping_transfer_function(x, min_out, max_out):
    return max(min_out, min(x, max_out))


def sigmoid_transfer_function(x, k=0.5):
    '''
    Classic Sigmoid shaper
    
    k is saturation amount
    '''
    return 2 * (1 / (1 + math.exp(-k * x))) - 1


def hyperbolic_tangent_function(x, k=0.5):
    return math.tanh(k * x) / math.tanh(k)


class Waveshaper:

    def __init__(self, transfer_fn=identity_transfer_function):
        self.transfer_fn = transfer_fn


    def __call__(self, val):
        return self.transfer_fn(val)


class SimpleDelay:

    def __init__(self, time, fdbk, wet_dry_ratio = 0.5, max_delay = 2.0, sample_rate=44100):
        self.delay_samples = int(time * sample_rate)
        self.max_delay_samples = int(max_delay * sample_rate)
        self.delay_buffer = np.zeros(self.max_delay_samples)
        self.fdbk = fdbk
        self.wet_dry_ratio = wet_dry_ratio
        self.sample_rate = sample_rate

        self.write_pos = 0

    def __call__(self, val):
        read_pos = self.write_pos - self.delay_samples
        if read_pos < 0:
            read_pos += self.max_delay_samples

        delayed_val = self.delay_buffer[read_pos]
        self.delay_buffer[self.write_pos] = val + self.fdbk * delayed_val
        self.write_pos = (self.write_pos + 1) % self.max_delay_samples
        return (1.0 - self.wet_dry_ratio) * val + self.wet_dry_ratio * delayed_val 


if __name__ == "__main__":
    
    sample_rate = 44100
    simple_delay = SimpleDelay(0.5, 0.9, 0.5, max_delay=2.0)

    for i in range(20):
        val = i % 2
        val = simple_delay(val)
        print("i: {}, val: {}, array: {}".format(i, val, simple_delay.delay_buffer))

    import os, sys
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))

    from synthesis.chain import Chain
    from synthesis.envelope import ADSREnvelope
    from synthesis.oscillator import SineOscillator
    from synthesis.modulated_oscillator import ModulatedOscillator, amp_mod

    mod_osc = ModulatedOscillator(SineOscillator(), ADSREnvelope(0.05, 0.3, 0.0, 0.0), amp_mod=amp_mod)
    iter(mod_osc)

    import utils_functions as UF
    UF.plot_sigspectrum(mod_osc.getNextBlockCallback(44100), fslice=slice(90,800))

    delay_chain = Chain(mod_osc, simple_delay)
    iter(delay_chain)

    import utils_functions as UF
    UF.plot_sigspectrum(delay_chain.getNextBlockCallback(20 * sample_rate), fslice=slice(90,800))

    from realtime_player import stream_audio
    iter(delay_chain)
    stream_audio(delay_chain.getNextBlockCallback)

