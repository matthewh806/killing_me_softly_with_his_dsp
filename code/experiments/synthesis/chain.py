
class Chain:
    '''
    Composition class for combining a single generator class in a serial chain
    with an arbitrary number of chain modifiers (e.g. sfx transformers)

    The modifier should be a callable (and sometimes an iterable?)
    '''
    def __init__(self, generator, *modifiers):
        self.generator = generator
        self.modifiers = modifiers

    def __getattr__(self, attr):
        '''
        Searches in the chain members for the desired attribute
        Note: This search concludes and returns the detected value
              on the first class member this attribute is found on
            
              First it checks the generator and then loops through
              the modifiers, so order is important
        '''
        val = None

        if hasattr(self.generator, attr):
            val = getattr(self.generator, attr)
        else:
            for mod in self.modifiers:
                if hasattr(mod, attr):
                    val = getattr(mod, attr)
                    break
            else:
                raise AttributeError("Attribute '{}' does not exist ".format(attr))
        return val

    def getNextBlockCallback(self, num_samples):
        return [next(self) for i in range(num_samples)]

    def trigger_release(self):
        tr = "trigger_release"

        if hasattr(self.generator, tr):
            self.generator.trigger_release()

        for mod in self.modifiers:
            if hasattr(mod, tr):
                mod.trigger_release()

    @property
    def ended(self):
        ended = []; e = "ended"

        if hasattr(self.generator, e):
            ended.append(self.generator.ended)
        ended.extend([m.ended for m in self.modifiers if hasattr(m, e)])

        if len(ended) == 0:
            return False

        return all(ended)

    def __iter__(self):
        iter(self.generator)
        # [iter(mod) for mod in self.modifiers if hasattr(mod, "__iter__")]
        return self

    def __next__(self):
        val = next(self.generator)
        
        for modifier in self.modifiers:
            val = modifier(val)
        
        return val

if __name__ == "__main__":
    import sys, os
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../effects'))
    from oscillator import SineOscillator
    from effects import sfx
    import utils_functions as UF

    osc_freq = 20
    chain = Chain(SineOscillator(osc_freq), sfx.Bitcrusher(5))
    signal = chain.getNextBlockCallback(44100)

    UF.plot_sigspectrum(signal, fslice=slice(90,800), sig_title="Signal: {} Hz".format(osc_freq), spec_title="Spectrum: {} Hz".format(osc_freq))

    from realtime_player import stream_audio
    iter(chain)
    stream_audio(chain.getNextBlockCallback)
