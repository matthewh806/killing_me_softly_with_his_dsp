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

class Waveshaper:

    def __init__(self, transfer_fn=identity_transfer_function):
        self.transfer_fn = transfer_fn

    def __call__(self, val):
        return self.transfer_fn(val)
