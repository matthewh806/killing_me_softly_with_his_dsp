
class ModulatedOscillator:
    '''
    A modulated oscillator class. The  basic idea of this class is to provide an oscillator
    which will be modulated by one or more modulators which are specified with the named 
    arguments modulator_mod which are expected to be a function with two parameters: initial val & env value
    '''
    def __init__(self, oscillator, *modulators, amp_mod=None, freq_mod=None, phase_mod=None):
        self.oscillator = oscillator
        self.modulators = modulators # list of max 3 modulators in order amp, freq, phase
        self.amp_mod = amp_mod
        self.freq_mod = freq_mod
        self.phase_mod = phase_mod

        self._num_modulators = len(modulators)

    def trigger_release(self):
        tr = "trigger_release"

        for modulator in self.modulators:
            if hasattr(modulator, tr):
                modulator.trigger_release()

        if hasattr(self.oscillator, tr):
            self.oscillator.trigger_release()

    def getNextBlockCallback(self, num_samples):
        return [next(self) for i in range(num_samples)]

    def _modulate(self, mod_values):
        if self.amp_mod is not None and self._num_modulators > 0:
            self.oscillator.amp = self.amp_mod(self.oscillator.initial_amp, mod_values[0])

        if self.freq_mod is not None:
            if self._num_modulators == 2:
                self.oscillator.freq = self.freq_mod(self.oscillator.initial_freq, mod_values[1])
            elif self._num_modulators == 1:
                self.oscillator.freq = self.freq_mod(self.oscillator.initial_freq, mod_values[0])

        if self.phase_mod is not None:
            if self._num_modulators == 3:
                self.oscillator.phase = self.phase_mod(self.oscillator.initial_phase, mod_values[2])
            elif self._num_modulators > 0:
                self.oscillator.phase = self.phase_mod(self.oscillator.initial_phase, mod_values[-1])

    @property
    def ended(self):
        e = "ended"
        ended=[]

        for modulator in self.modulators:
            if hasattr(modulator, e):
                ended.append(modulator.ended)

        if hasattr(self.oscillator, e):
            ended.append(self.oscillator.ended)
        
        return all(ended)

    def __iter__(self):
        iter(self.oscillator)
        [iter(modulator) for modulator in self.modulators]

        return self

    def __next__(self):
        mod_vals = [next(modulator) for modulator in self.modulators]
        self._modulate(mod_vals)

        return next(self.oscillator)

def amp_mod(init_amp, env):
    return env * init_amp

def freq_mod(init_freq, env, mod_amt=0.01, sustain_level=0.7):
    return init_freq + ((env - sustain_level) * init_freq * mod_amt)

def get_down_len(env, sustain_len, sample_rate=44100.0):
    time = sum(env.attack_duration, env.release_duration, sustain_len)
    return int(time * sample_rate)

def get_trig(gen, downtime, sample_rate=44100.0):
    gen = iter(gen)
    down_samples = int(downtime * sample_rate)
    values = gen.getNextBlockCallback(down_samples)
    gen.trigger_release()

    while not gen.ended:
        values.append(next(gen))

    return values

def get_signal(mod_osc, downtime):
    return get_trig(mod_osc, downtime)

if __name__ == "__main__":
    from oscillator import SquareOscillator
    from envelope import ADSREnvelope
    import os, sys
    sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
    from realtime_player import stream_audio

    a, d, sl, r = 1.0, 0.01, 0.3, 0.3 
    sd = 0.4

    mod_osc = ModulatedOscillator(
        SquareOscillator(110.0),
        ADSREnvelope(a,d,sl,r),
        amp_mod=amp_mod,
        freq_mod=lambda init_freq, env_value: freq_mod(init_freq, env_value, mod_amt=0.1, sustain_level=sl),
        phase_mod=lambda init_phase, env_value: freq_mod(init_phase, env_value, mod_amt=1.0, sustain_level=sl)
    )

    downtime=a+d+sd

    signal = get_signal(mod_osc, downtime)
    
    import matplotlib.pyplot as plt

    fig = plt.figure(figsize=(25, 6.25))
    plt.title("Synth with Envelope")

    plt.plot(signal)

    plt.ylabel("amp")
    plt.xlabel("samples")
    plt.grid()
    plt.show()

    iter(mod_osc)
    stream_audio(mod_osc.getNextBlockCallback)
