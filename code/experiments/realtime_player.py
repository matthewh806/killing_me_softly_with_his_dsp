import pyaudio
import numpy as np
import time
from threading import Timer
from synthesis.oscillator import SineOscillator
import signal
import sys

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
        Open the pyaudio stream ready for audio output. 
        get_next_block_callback is called eachtime the pyaudio stream_callback is called (this is determined by the rate
        parameter, which in this case is just sample_rate) it should return a buffer of size FRAMES_PER_BUFFER

        If sample_rate is 44100 Hz then get_next_block_callback is called 44100 times per second and should fill 
        FRAMES_PER_BUFFER worth of data

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
            
        self.stream.close()
        self.stream = None
        self.cycle_count = 0


    def _pyaudio_callback(self, in_data, frame_count, time_info, status):
        # read in new data via callback
        outdata = np.zeros(FRAMES_PER_BUFFER)

        if(self.get_next_block_callback != None):
            outdata = self.get_next_block_callback(FRAMES_PER_BUFFER)

        audio_data = np.float32(outdata) 
        self.cycle_count += 1
        return (np.float32(audio_data).tobytes(), pyaudio.paContinue)


    def _on_complete_check(self):
        while self.processing():
            time.sleep(0.1)
        
        print("Stopping processing")
        self.stop_processing()


def stream_audio(callback_method, sample_rate=44100):
    sound_player = RealtimePlayer()

    def signal_handler(sig, frame):
        print("Closing any open streams")
        if sound_player.processing():
            sound_player.stop_processing()

        sound_player.paudio.terminate()
        sys.exit()

    signal.signal(signal.SIGINT, signal_handler)
    print("Press Ctrl+C to terminate")

    sound_player.start_processing_non_blocking([], sample_rate, callback_method)

    return sound_player

if __name__ == "__main__":
    import utils_functions as UF
    import time
    import sys

    osc=SineOscillator(440, 1.0)
    stream_audio(osc.getNextBlockCallback)
