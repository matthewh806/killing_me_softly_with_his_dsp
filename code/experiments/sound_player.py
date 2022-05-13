import pyaudio
import numpy as np
import time
from threading import Timer

class NumpySoundPlayer:
    '''
    A simple sound player class based around pyaudio for streaming a 
    numpy array to the default sound device

    Note: 
        - Works only for single channel data
        - expects the numpy array to be in float32
        - block size is fixed at 1024

    TODO: The pyaudio API & C bindings don't expose the PaStreamFinishedCallback 
          see: http://files.portaudio.com/docs/v19-doxydocs/portaudio_8h.html#ab2530ee0cb756c67726f9074d3482ef2

          This means the calling class cannot be informed of when the stream finishes, this is useful and desirable
          for e.g. updating UI states etc. To get around this I've implemented a Timer thread which simply polls for
          the audio stream status every 0.1 seconds until stream.is_active() is False, once this condition is met
          if the on_complete_callback is set it will fire.
          This is not the most robust or efficient solution - it would be better to fork the pyaudio repository 
          and adapt the bindings to support this functionality
    '''

    def __init__(self):
        self.paudio = pyaudio.PyAudio()
        self.cycle_count = 0
        self.on_complete_callback=None
        self.timer_thread = None


    def start_processing_non_blocking(self, sound_array, sample_rate=44100.0, on_complete_callback=None):
        '''
        Start streaming the audio contained in sound_array using the provided
        sample rate

        Use on_complete_callback to be notified of when the audio playback has completed.

        In order to terminate the before the end processing call stop_processing()
        Note this process is non-blocking meaning it will be called on a different thread 
        to the main one
        '''
        self.sound_source = sound_array
        self.on_complete_callback = on_complete_callback
        self.stream = self.paudio.open(format=pyaudio.paFloat32, 
                                        channels=1, 
                                        rate=sample_rate, 
                                        frames_per_buffer=1024, 
                                        input=False,
                                        output=True, 
                                        output_device_index=None, 
                                        stream_callback=self._pyaudio_callback,
                                        stream_complete_callback=on_complete_callback)

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
        audio_len = len(self.sound_source)

        if frame_count * self.cycle_count > audio_len:
            self.cycle_count = 0

            return (None, pyaudio.paComplete)

        start_pos = frame_count * self.cycle_count
        end_pos = start_pos + frame_count

        remaining_frames = len(self.sound_source) - end_pos
        audio_data = self.sound_source[start_pos:end_pos]

        if remaining_frames < frame_count:
            pad_length = frame_count - remaining_frames
            audio_data = np.pad(audio_data, (0, pad_length))

        audio_data = np.float32(audio_data)
        self.cycle_count += 1

        return (np.float32(audio_data).tobytes(), pyaudio.paContinue)


    def _on_complete_check(self):
        while self.processing():
            time.sleep(0.1)

        if self.on_complete_callback is not None:
            self.on_complete_callback()
        
        self.stop_processing()


if __name__ == "__main__":
    import utils_functions as UF
    import time

    sample_rate = 44100
    sine_wave = UF.generateSineSignal(440.0, 3.0, sample_rate)

    def on_complete_callback():
        print("Finished Processing callback")

    sound_player = NumpySoundPlayer()
    sound_player.start_processing_non_blocking(sine_wave, sample_rate, on_complete_callback=on_complete_callback)

    while sound_player.processing():
        time.sleep(0.1)