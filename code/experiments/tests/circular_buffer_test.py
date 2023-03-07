import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.lines import Line2D
import random
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF

'''
Read static sine signal over time into a circular buffer 
and read from this buffer into an output buffer

Just to visualise how the circular buffer works

Inspired by wanting to recreate this animated plot: https://youtu.be/uX-FVtQT0PQ?t=424
'''

if __name__  == "__main__":
    # This is just to see the effect of delaying the read operation by OFFSET_READ frames
    OFFSET_READ = 37

    sample_rate = 1000
    signal = UF.generateSineSignal(10, 0.4, sample_rate)

    pos_signal = 0 # The current sample position we're reading from in the signal itself (into circular buffer)
    len_signal = len(signal)

    circular_buffer = UF.CircularBuffer(50)

    output_buffer = np.zeros(len(signal))
    pos_output = 0 # The current sample position we're writing to in the output signal (from the circular buffer)

    circ_xdata = np.arange(0, circular_buffer.size)
    circ_ydata = np.zeros(circular_buffer.size)

    output_xdata = np.arange(0, len(output_buffer))
    output_ydata = np.zeros(len(output_buffer))

    fig = plt.figure(figsize=(10, 10))
    fig.tight_layout()
    ax1 = plt.subplot2grid(shape=(3, 3), loc=(0, 0), colspan=3)
    line1, = ax1.plot(signal)
    signal_pos_line = Line2D([], [], color='red', linewidth=2)
    ax1.add_line(signal_pos_line)
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Amp")
    ax1.set_title("Signal")
    ax1.grid()

    ax2 = plt.subplot2grid(shape=(3, 3), loc=(1, 0), colspan=1)
    line2, = ax2.plot(circ_xdata, circ_ydata)
    write_pos_line = Line2D([], [], color='red', linewidth=2)
    read_pos_line = Line2D([], [], color='blue', linewidth=2)
    ax2.add_line(write_pos_line)
    ax2.add_line(read_pos_line)
    ax2.set_xlim(0, circular_buffer.size)
    ax2.set_xlabel("Sample")
    ax2.set_ylabel("Amp")
    ax2.set_title("Circular Buffer (size={})".format(circular_buffer.size))
    ax2.grid()

    ax3 = plt.subplot2grid(shape=(3, 3), loc=(2, 0), colspan=3)
    line3, = ax3.plot(output_ydata)
    ax3.set_xlabel("Sample")
    ax3.set_ylabel("Amp")
    ax3.set_title("Output Signal")

    def init():
        ax1.set_ylim(-1, 1)
        ax2.set_ylim(-1, 1)
        ax3.set_ylim(-1, 1)

        signal_pos_line.set_data([pos_signal, pos_signal], [-1,1])
        write_pos_line.set_data([circular_buffer.head, circular_buffer.head], [-1,1])
        read_pos_line.set_data([circular_buffer.tail, circular_buffer.tail], [-1,1])

        circ_ydata = np.zeros(circular_buffer.size)
        line2.set_data(circ_xdata, circ_ydata)

        output_ydata = np.zeros(len(output_buffer))
        line3.set_data(output_xdata, output_ydata)

        return signal_pos_line, line2, line3, write_pos_line, read_pos_line

    def run(data):
        global pos_signal 
        global pos_output

        # Get the current signal value
        pos_signal = (pos_signal + 1) % len_signal
        signal_value = signal[pos_signal]

        # Write into the value circular buffer 
        circular_buffer.write(signal_value)
    
        output_value = circular_buffer.read() if pos_signal > OFFSET_READ else 0
        pos_output = (pos_output + 1) % len(output_xdata) if pos_signal > OFFSET_READ else 0

        # Update the plotting structures
        signal_pos_line.set_data([pos_signal, pos_signal], [-1, 1])
        write_pos_line.set_data([circular_buffer.head, circular_buffer.head], [-1,1])
        read_pos_line.set_data([circular_buffer.tail, circular_buffer.tail], [-1,1])
        circ_ydata[circular_buffer.head] = signal_value
        output_ydata[pos_output] = output_value

        line2.set_ydata(circ_ydata)
        line3.set_ydata(output_ydata)

        return signal_pos_line, line2, line3, write_pos_line, read_pos_line

    ani = animation.FuncAnimation(fig, run, blit=False, interval=100, 
                                  repeat=False, init_func=init)

    plt.show()
