import matplotlib.pyplot as plt
import numpy as np
import utils_functions as UF
import math
from scipy.io.wavfile import write

'''
Exploration of a synthesis technique described in: https://nathan.ho.name/posts/ixa-synthesis/

Based on a type of synthesis called "IXA Sound Source" created for the Casio CTK-1000. The approach is 
based on a modulation synthesis method with a triangle carrier wave and a sine wave modulator. 

The phase of triangle wave is distorted s.t. a sine wave is produced if the modulation index is 0

IXA(n,r,t) = Tri(W(t) + n*sin(2*pi*r*t))

n is index, r is ratio, assuming period=1

W(t) = (2 * Pulse_1/2(t) - 1) * sin(2*pi*mod(t,1/2)) + 2 * Pulse_1/2(t + 1/4) + 2 * Pulse_1(t+1/2)

where Pulse_x(t) is a pulsewave with period x
'''

def pulseValue(t, period):
    '''
    Pulse_x(t) = 1 if 0 <= t < period/2,
    Pulse_x(t) = 0 if period / 2 <= t < period
    '''
    t %= period
    return 1 if 0 <= t and t < period/2 else 0

def triangleValue(t, period):
    t %= period

    if 0 <= t and t < period / 4:
        return t
    elif period / 4 <= t and t < 3 * period / 4:
        return 2-t

    return t-4

def weirdWaveValue(t):
        return (2*pulseValue(t, 0.5)-1)*np.sin(2*np.pi * math.fmod(t, 0.5)) + 2*pulseValue(t+0.25, 0.5) + 2*pulseValue(t+0.5, 1)

def generatePulseWave(length, period, sample_rate=44100):
    t=0
    inc=(length/period)/sample_rate
    samples = length * sample_rate
    
    wave=[]
    for i in range(samples):
        wave.append(pulseValue(t, period))
        t+=inc 

    return np.array(wave)

def generateTriangleWave(length, period, sample_rate=44100):
    t=0
    inc=(length/period)/sample_rate
    samples = length * sample_rate

    wave=[]
    for i in range(samples):
        wave.append(triangleValue(t, period))
        t+=inc

    return np.array(wave)


def generateWeirdWave(length, period, sample_rate=44100):
    t=0
    inc=(length/period)/sample_rate
    samples = length * sample_rate

    wave=[]
    for i in range(samples):
        wave.append(weirdWaveValue(t))
        t+=inc
    
    return np.array(wave)


def generateIXAWave(length, period, index, ratio, sample_rate=44100):
    t = 0
    inc=period/sample_rate
    samples = length*sample_rate

    wave=[]
    for i in range(samples):
        wave.append(triangleValue(weirdWaveValue(t) + index*np.sin(2*ratio*t), 4))
        t+=inc

    return np.array(wave)


if __name__ == "__main__":
    sample_rate=44100

    pulse_times = np.linspace(0, 1, sample_rate)
    pulse_wave = generatePulseWave(1, 1, sample_rate)
    UF.signal_plot(4,1,1, pulse_times, pulse_wave, "Pulse Wave")

    triangle_times = np.linspace(0, 4, 4*sample_rate)
    triangle_wave = generateTriangleWave(4, 4.0, sample_rate)
    UF.signal_plot(4,1,2, triangle_times, triangle_wave, "Triangle Wave")
    
    weird_times = np.linspace(0, 1.0, sample_rate)
    weird_wave = generateWeirdWave(1, 1, sample_rate)
    UF.signal_plot(4,1,3, weird_times, weird_wave, "Weird Wave")

    ixa_times =  np.linspace(0, 4.0, 4*sample_rate)
    ixa_wave = generateIXAWave(4, 1.0, 1.0, 1)
    UF.signal_plot(4,1,4, ixa_times, ixa_wave)
    plt.show()

    write("ixa_wave.wav", int(sample_rate), ixa_wave)
