import copy
import PySimpleGUI as sg
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib
matplotlib.use('TkAgg')
import numpy as np
import gc
import matplotlib.pyplot as plt
import time

import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../synthesis'))
from synthesis.oscillator import SineOscillator, SquareOscillator
from synthesis.modulated_oscillator import ModulatedOscillator
import utils_functions as UF
import realtime_player

'''
This is a simple test to investigate how to make a simple GUI with
plotting using the PySimplyGUI library. 
'''

def draw_canvas_figure(canvas, figure):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

class FMSynthGUI():
    def __init__(self, synth):
        self.synth = synth
        self.player = None

        self.plot_signal = np.zeros(44100)
        self.plot_signal_write_pos = 0
        self.replot = False

        iter(self.synth)

        sg.theme('DarkAmber')

        layout = [[sg.Text('Oscillator')],
                [sg.Button('Play'), sg.Button("Stop")],
                [
                    sg.Text("Carrier Amplitude:" ), 
                    sg.Slider(range=(0, 100), default_value = self.synth.oscillator.amp, size=(30,20), orientation = "horizontal", key="-C-AMP-", enable_events = True), 
                    sg.Text('' * 2),
                    sg.Text("Modulator Amplitude:" ), 
                    sg.Slider(range=(0, 100), default_value = self.synth.modulators[0].amp, size=(30,20), orientation = "horizontal", key="-M-AMP-", enable_events = True), 
                    sg.Text('')
                ],
                [
                    sg.Text("Carrier Frequency:" ), 
                    sg.Slider(range=(0, 1200), default_value = self.synth.oscillator.freq, size=(30,20), orientation = "horizontal", key="-C-FREQ-", enable_events = True), 
                    sg.Text(' Hz'),
                    sg.Text("Modulator Frequency:" ), 
                    sg.Slider(range=(0, 1200), default_value = self.synth.modulators[0].freq, size=(30,20), orientation = "horizontal", key="-M-FREQ-", enable_events = True), 
                    sg.Text(' Hz')
                ],
                [
                    sg.Text("Carrier Phase:" ), 
                    sg.Slider(range=(0, 1), default_value = self.synth.oscillator.phase, size=(30,20), orientation = "horizontal", key="-C-PHASE-", enable_events = True), 
                    sg.Text(''),
                    sg.Text("Modulator Phase:" ), 
                    sg.Slider(range=(0, 1), default_value = self.synth.modulators[0].phase, size=(30,20), orientation = "horizontal", key="-M-PHASE-", enable_events = True), 
                    sg.Text('')
                ],
                [sg.Canvas(key="-CANVAS-", size=(600,400))],
                [sg.Button('Plot')]
        ]

        self.window = sg.Window("FM Synth", layout, finalize=True)
        self.canvas = self.plot_figure()

    def poll_events(self, timeout=None):
        event, values = self.window.read(timeout)
        return event, values

    def plot_figure(self):
        self.plot_signal = self.synth.getNextBlockCallback(44100)
        self.fig = UF.get_sigspectrum(self.plot_signal, fslice=slice(0, 1000), figsize=(6,4))
        return draw_canvas_figure(self.window['-CANVAS-'].TKCanvas, self.fig)

    def update_figure(self):
        mod_copy = copy.deepcopy(self.synth)
        self.plot_signal = mod_copy.getNextBlockCallback(44100)

        axes = self.fig.axes
        sig_lines = axes[0].get_lines()
        sig_lines[0].set_ydata(self.plot_signal)

        spec_lines = axes[1].get_lines()
        fx, fy = UF.fft_plot_data(self.plot_signal, fslice=slice(0, 1000))
        spec_lines[0].set_xdata(fx)
        spec_lines[0].set_ydata(fy)

        self.fig.canvas.draw_idle()
        self.replot = False

    def run_gui(self):

        start_time = time.time()
        time_limit = 1

        while True:
            event, values = self.poll_events(timeout=10)

            current_time = time.time()
            elapsed_time = current_time - start_time

            if elapsed_time > time_limit:
                start_time = current_time
                if self.replot:
                    self.update_figure()

            if event == sg.WIN_CLOSED or event == 'Exit':
                if self.player is not None:
                    self.player.stop_processing()
                    self.player = None
                break
            
            if event == "-C-AMP-":
                self.synth.oscillator.amp = values['-C-AMP-']/100.0
                self.replot = True
            
            if event == "-M-AMP-":
                self.synth.modulators[0].amp = values['-M-AMP-']/100.0
                self.replot = True

            if event == "-C-FREQ-":
                self.synth.oscillator.freq = values['-C-FREQ-']
                self.replot = True

            if event == "-M-FREQ-":
                self.synth.modulators[0].freq = values['-M-FREQ-']
                self.replot = True
            
            if event == "-C-PHASE-":
                self.synth.oscillator.phase = values['-C-PHASE-']
                self.replot = True
            
            if event == "-M-PHASE-":
                self.synth.modulators[0].phase = values['-M-PHASE-']
                self.replot = True

            if event == 'Plot':
                self.replot = True

            if event == 'Play':
                if self.player is not None:
                    self.player.stop_processing()
                    self.player = None

                self.player = realtime_player.stream_audio(self.synth.getNextBlockCallback)

            if event == 'Stop' and self.player is not None:
                self.player.stop_processing()
                self.player = None

        self.window.close()

def freq_mod(init_freq, val):
    return init_freq * val
        

if __name__ == "__main__":
    fmsynth = ModulatedOscillator(
        SineOscillator(freq=440),
        SineOscillator(freq=0.1, wave_range=(0.2, 1)),
        freq_mod=freq_mod    
    )

    fmsynthGUI = FMSynthGUI(fmsynth)
    fmsynthGUI.run_gui()
