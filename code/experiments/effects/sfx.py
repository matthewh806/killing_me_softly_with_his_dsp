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
