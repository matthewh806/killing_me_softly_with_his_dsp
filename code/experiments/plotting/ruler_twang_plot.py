import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import physical_modelling.ruler_twang as ruler_twang
import matplotlib.pyplot as plt
import numpy as np
import math

'''
Basic graphing of the Beam equation

Shows the change in amplitude over time for a vibrating ruler
'''

if __name__ == "__main__":
    
    E = 7.05e10 # Youngs Modulus
    h = 0.003   # height of beam
    l = 0.5 # length of beam
    rho = 10000 # mass density
    lamb = 4.7300407

    w = ruler_twang.get_angular_frequency(E, h, l, rho, lamb)
    print("Angular Frequency: {}, Frequency: {}".format(w, w / (2.0 * math.pi)))

    t = np.linspace(0, 0.5, 100)
    x=l
    y = ruler_twang.get_vertical_displacement(t, x, w, E, h / math.sqrt(12), rho, 0.0, 10)

    plt.subplot(1, 1, 1)
    plt.plot(t, y)
    plt.title("TWAAAAAAANNG")
    plt.grid()
    plt.xlabel("t [s]")
    plt.ylabel("Amplitude [m]")
    plt.show()
