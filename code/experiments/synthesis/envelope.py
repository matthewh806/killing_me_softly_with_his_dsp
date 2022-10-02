
import itertools


class  ADSREnvelope:
    def __init__(self, attack_dur=0.05, decay_dur=0.2, sustain_level=0.7, release_dur=0.3, sample_rate=44100):
        self.attack_dur = attack_dur
        self.decay_dur = decay_dur
        self.sustain_level = sustain_level
        self.release_dur = release_dur
        self.sample_rate = sample_rate

    def get_ads_stepper(self):
        '''
        This function generates the output value for the envelope when
        in the attack, decay or sustain portions
        '''
        steppers=[]

        if self.attack_dur > 0:
            steppers.append(itertools.count(start=0, step=1/(self.attack_dur * self.sample_rate)))
        if self.decay_dur > 0:
            steppers.append(itertools.count(start=1, step=-(1-self.sustain_level) / (self.decay_dur * self.sample_rate)))

        while True:
            l = len(steppers)
            if l > 0:
                val = next(steppers[0])

                if l==2 and val > 1:
                    # attack stage complete
                    steppers.pop(0)
                    val = next(steppers[0])
                elif l==1 and val < self.sustain_level:
                    # decay stage complete
                    steppers.pop(0)
                    val = self.sustain_level
            else:
                val = self.sustain_level

            yield val


    def get_r_stepper(self):
        '''
        This function generates the output value for the envelope when 
        in the release portion
        '''
        val = 1

        if self.release_dur > 0:
            stepper = itertools.count(self.val, step=-self.val / (self.release_dur *  self.sample_rate))
        else:
            val = -1

        while True:
            if val <= 0:
                self.ended = True
                val = 0
            else:
                val = next(stepper)

            yield val

    def trigger_release(self):
        '''
        Move the state of the envelope to the release stage. 
        (This is not maintained directly but simply changes the stepper)
        '''
        self.stepper = self.get_r_stepper()

    def __iter__(self):
        self.val = 0
        self.ended = False
        self.stepper = self.get_ads_stepper()

        return self

    def __next__(self):
        self.val = next(self.stepper)
        return self.val 

if __name__ == "__main__":

    def _get_adsr_envelope(a=0.5,  d=0.3, sl=0.7, r=0.2, sd=0.4, sr=44100):
        # sd = sustain duration
        adsr_generator = ADSREnvelope(a, d, sl, r, sr); iter(adsr_generator)

        ads_samples = int(sum([a, d, sd]) * sr)
        release_samples = int(r * sr)

        adsr_envelope = [next(adsr_generator) for _ in range(ads_samples)]
        adsr_generator.trigger_release()
        adsr_envelope.extend([next(adsr_generator) for _ in range(release_samples)])

        return adsr_envelope

    env = _get_adsr_envelope()
    import matplotlib.pyplot as plt

    fig = plt.figure(figsize=(25, 6.25))
    plt.title("ADSR Envelope")

    plt.plot(env)

    plt.ylabel("amp")
    plt.xlabel("samples")
    plt.grid()
    plt.show()
