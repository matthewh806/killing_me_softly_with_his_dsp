import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import physical_modelling.ruler_twang as ruler_twang
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import math

'''
Animated plot of the beam equation

Shows the motion of the ruler over time in an animated plot
'''

if __name__ == "__main__":
    
    E = 9e10 # Youngs Modulus
    h = 0.003   # height of beam
    l = 1 # length of beam
    rho = 1350 # mass density
    lamb = 4.7300407

    w = ruler_twang.get_angular_frequency(E, h, l, rho, lamb)
    print("Angular Frequency: {}, Frequency: {}".format(w, w / (2.0 * math.pi)))

    t = 0
    x = np.linspace(0, l, 100)
    y = ruler_twang.get_vertical_displacement(t, x, w, E, h / math.sqrt(12), rho, 0.0, 1.0)

    fig = plt.figure(figsize=(10,10))
    fig.tight_layout
    ax1 = plt.subplot2grid(shape=(1, 1), loc=(0,0))
    ax1.set_ylim(-10, 10)
    ax1.set_xlabel("x [m]")
    ax1.set_ylabel("y [m]")
    ax1.set_title("TWAAAAAANG!")
    line1, = ax1.plot(x, y)

    def run(data):
        global t

        t += (0.1 / 1000)
        y = ruler_twang.get_vertical_displacement(t, x, w, E, h / math.sqrt(12), rho, 0.0, 0.1)
        line1.set_ydata(y)

    ani = animation.FuncAnimation(fig, run, blit=False, interval=10, repeat=False)
    plt.show()

