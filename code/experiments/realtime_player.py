import itertools
import pyaudio
import numpy as np
import time
from threading import Timer

FRAMES_PER_BUFFER=1024
class RealtimePlayer:
    '''
    A simple sound player class based around pyaudio for streaming a 
    numpy array in real time

    Note: 
        - Works only for single channel data
        - expects the numpy array to be in float32
        - block size is fixed at 1024

    TODO: KILL STREAM WITH KEY PRESS / PROGRAM EXIT
    '''
    

    def __init__(self):
        self.paudio = pyaudio.PyAudio()
        self.cycle_count = 0
        self.timer_thread = None

    def start_processing_non_blocking(self, sound_array, sample_rate=44100.0, get_next_block_callback=None):
        '''
        Start streaming the audio contained in sound_array using the provided
        sample rate

        Use on_complete_callback to be notified of when the audio playback has completed.

        In order to terminate the before the end processing call stop_processing()
        Note this process is non-blocking meaning it will be called on a different thread 
        to the main one
        '''
        self.sound_source = sound_array
        self.stream = self.paudio.open(format=pyaudio.paFloat32, 
                                        channels=1, 
                                        rate=sample_rate, 
                                        frames_per_buffer=FRAMES_PER_BUFFER, 
                                        input=False,
                                        output=True, 
                                        output_device_index=None, 
                                        stream_callback=self._pyaudio_callback)

        self.get_next_block_callback=get_next_block_callback
        self.sample_rate = sample_rate
        self.stream.start_stream()
        self.timer_thread = Timer(0.1, self._on_complete_check)
        self.timer_thread.start()


    def processing(self):
        if not hasattr(self, "stream"):
            return False

        if self.stream is None:
            return False

        return self.stream.is_active()
        

    def stop_processing(self):
        '''
        Abort the audio streaming if currently running
        '''
        self.timer_thread.cancel()

        if not hasattr(self, "stream"):
            return

        if self.stream is None:
            return False

        if not self.stream.is_stopped():
            self.stream.stop_stream()
            
        self.stream.close()
        self.stream = None

        self.cycle_count = 0
        self.sound_source = np.array([], dtype=np.float32)


    def _pyaudio_callback(self, in_data, frame_count, time_info, status):
        # read in new data via callback
        outdata = np.zeros(FRAMES_PER_BUFFER)

        if(self.get_next_block_callback != None):
            outdata = self.get_next_block_callback()

        audio_data = np.float32(outdata) 
        self.cycle_count += 1
        return (np.float32(audio_data).tobytes(), pyaudio.paContinue)


    def _on_complete_check(self):
        while self.processing():
            time.sleep(0.1)
        
        print("Stopping processing")
        self.stop_processing()

def get_sine_oscillator(freq, sample_rate):
    increment = (2*np.pi*freq)/sample_rate
    return (np.sin(v) for v in itertools.count(start=0, step=increment))

class SineOscillator():
    def __init__(self, freq, sample_rate):
        self.freq = freq
        self.sample_rate = sample_rate
        self.osc = self._get_oscillator()
    
    def _get_oscillator(self):
        increment = (2*np.pi*self.freq)/self.sample_rate
        return (np.sin(v) for v in itertools.count(start=0, step=increment))

    def getNextBlockCallback(self):
        return [next(self.osc) for i in range(FRAMES_PER_BUFFER)]

if __name__ == "__main__":
    import utils_functions as UF
    import time

    sample_rate = 44100

    osc=SineOscillator(440, sample_rate)
    def getNextBlockCallback(self):
        return [next(self.oscillator) for i in range(FRAMES_PER_BUFFER)]
    
    sound_player = RealtimePlayer()
    sound_player.start_processing_non_blocking([], sample_rate, osc.getNextBlockCallback)

    while sound_player.processing():
        time.sleep(0.1)

    print("Finished playing")
