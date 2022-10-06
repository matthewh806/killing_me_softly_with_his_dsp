import PySimpleGUI as sg
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib
matplotlib.use('TkAgg')
import numpy as np
import gc
import matplotlib.pyplot as plt

import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../synthesis'))
from synthesis.oscillator import SineOscillator, SquareOscillator
import utils_functions as UF
import realtime_player

'''
This is a simple test to investigate how to make a simple GUI with
plotting using the PySimplyGUI library. 

TODO:
    - plotting / audio rendering are done on the same thread & can
      result in big spikes in  the plots
    - Check for memory leaks
    - Improve matplotlib replotting?
'''

def draw_canvas_figure(canvas, figure):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

class OscillatorGUI():

    def __init__(self, osc):
        self.osc = osc
        self.player = None
        self.plot_signal = np.zeros(44100)
        iter(self.osc)

        sg.theme('DarkAmber')

        layout = [[sg.Text('Oscillator')],
                [sg.Button('Play'), sg.Button("Stop")],
                [
                    sg.Text("Amplitude:" ), 
                    sg.Slider(range=(0, 100), default_value = osc.amp, size=(30,20), orientation = "horizontal", key="-AMP-", enable_events = True), 
                    sg.Text('')
                ],
                [
                    sg.Text("Frequency:" ), 
                    sg.Slider(range=(0, 1200), default_value = osc.freq, size=(30,20), orientation = "horizontal", key="-FREQ-", enable_events = True), 
                    sg.Text(' Hz')
                ],
                [
                    sg.Text("Phase:" ), 
                    sg.Slider(range=(0, 1), default_value = osc.phase, size=(30,20), orientation = "horizontal", key="-PHASE-", enable_events = True), 
                    sg.Text('')
                ],
                [sg.Canvas(key="-CANVAS-", size=(600,400))],
                [sg.Button('Plot')]
        ]

        self.window = sg.Window("Basic Oscillator", layout, finalize=True)
        self.plot_signal = self.osc.getNextBlockCallback(44100)
        self.canvas = self.plot_figure()

    def poll_events(self):
        event, values = self.window.read()
        return event, values

    def plot_figure(self):
        self.plot_signal = self.osc.getNextBlockCallback(44100)
        self.fig = UF.get_sigspectrum(self.plot_signal, fslice=slice(0, 1000), figsize=(6,4))
        return draw_canvas_figure(self.window['-CANVAS-'].TKCanvas, self.fig)

    def update_figure(self):
        self.fig.clear()
        plt.close(self.fig)
        plt.close('all')

        self.canvas.get_tk_widget().forget()
        self.canvas = self.plot_figure()

        print(plt.get_fignums())

    def run_gui(self):
        while True:
            event, values = self.poll_events()

            if event == sg.WIN_CLOSED or event == 'Exit':
                if self.player is not None:
                    self.player.stop_processing()
                    self.player = None

                break
            
            if event == "-AMP-":
                self.osc.amp = values['-AMP-']/100.0
                self.update_figure()

            if event == "-FREQ-":
                self.osc.freq = values['-FREQ-']
                self.update_figure()
            
            if event == "-PHASE-":
                self.osc.phase = values['-PHASE-']
                self.update_figure()

            if event == 'Plot':
                self.update_figure()

            if event == 'Play':
                if self.player is not None:
                    self.player.stop_processing()
                    self.player = None

                self.player = realtime_player.stream_audio(self.osc.getNextBlockCallback)

            if event == 'Stop' and self.player is not None:
                self.player.stop_processing()
                self.player = None

        self.window.close()
        

if __name__ == "__main__":
    osc = SquareOscillator()
    iter(osc)
    oscGUI = OscillatorGUI(osc)
    oscGUI.run_gui()
