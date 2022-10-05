import PySimpleGUI as sg
import sys, os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../synthesis'))
from synthesis.oscillator import SineOscillator, SquareOscillator
import utils_functions as UF
import realtime_player

class OscillatorGUI():

    def __init__(self, osc):
        self.osc = osc
        self.player = None
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
                [sg.Button('Plot')]
        ]

        self.window = sg.Window("Basic Oscillator", layout)

    def poll_events(self):
        event, values = self.window.read()
        print(event, values)
        return event, values

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

            if event == "-FREQ-":
                self.osc.freq = values['-FREQ-']
            
            if event == "-PHASE-":
                self.osc.phase = values['-PHASE-']

            if event == 'Plot':
                signal = self.osc.getNextBlockCallback(44100)
                UF.plot_sigspectrum(signal, fslice=slice(0, 1000))

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
