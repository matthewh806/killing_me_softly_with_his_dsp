import math
import numpy as np

'''
Explorations of the Euler-Bernoulli beam equation:

d^2y/dt^2 + E*I/rho * d^4y/dx^4 = 0

E: youngs modulus
I: sectional moment
rho: mass density

Separating variables as y = f(x)g(t)

The general solution is:

y(x,t) = (Asin(kx) + Bcos(kx) + Csinh(kx) + Dcosh(kx)) * sin(wt + phi)

where k = ((w^2 * rho) / (E*I))^1/4 and w is angular frequency (w = 2*pi*f)

Refs: https://homepages.abdn.ac.uk/d.j.benson/pages/html/music.pdf
      http://www.eecs.qmul.ac.uk/~josh/documents/2022/Selfridge%20AES152.pdf
'''

def get_angular_frequency(E, h, l, rho, lamb):
    '''
    Calculates the angular frequency from:

        E - Youngs modulus (N/m^2)
        h - height of the ruler (m)
        l - length of the ruler (m) 
        rho - mass density of the ruler (kg / m^3)
        lamb - numerically determined constant (see refs)
    '''
    kappa = h / math.sqrt(12)
    return math.sqrt(E * kappa * kappa / rho) * lamb * lamb / (l*l)


def get_vertical_displacement(t, x, w, E, kappa, rho, phase, qfactor):
    '''
    Return the vertical displacements in y for a given array of t's and x's 
    
    t and x should be numpy arrays of length n
    phase is just a constant for now
    qfactor is used for determining the damping factor. Set to 0 for no damping!

    returned y is a numpy array of length n
    '''
    A = 1.0
    B = 0.5
    C = 0.25
    D = 0.125

    k = math.pow((w * w * rho) / (E * kappa * kappa), 0.25)
    alpha = w / (qfactor * 2) if qfactor > 0.0 else 0.0
    return (A*np.sin(k*x) + B*np.cos(k*x) + C*np.sinh(k*x) + D*np.cosh(k*x)) * np.sin(w*t + phase) * np.exp(-alpha * t)
