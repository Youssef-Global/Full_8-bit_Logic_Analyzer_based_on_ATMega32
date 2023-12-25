import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import tkinter as tk
import numpy as np
import serial
import time
import customtkinter
import threading

# Create the main window
frame = tk.Tk()
frame.title("Logic Analyzer")
frame.configure(background="white")

runningflag = True
samples = 255
new_samples = 255


def button_callback():
    global new_samples
    if samples_entry.get() != '':
        new_samples = int(samples_entry.get())
        if new_samples > 250:
            new_samples = 250
        elif samples < 25:
            new_samples = 25
        samples_entry.delete(0, tk.END)    


# Create a Frame to contain the Entry and Button widgets
entry_frame = tk.Frame(master=frame)
entry_frame.pack(side=customtkinter.RIGHT, expand=1)

# Add an Entry widget to the Frame
samples_entry = tk.Entry(master=entry_frame)
samples_entry.pack(side=tk.TOP, expand=1, pady=5)

# Add a Button widget to the Frame
send_button = tk.Button(master=entry_frame, text="Send Samples Number", command=button_callback)
send_button.pack(side=tk.TOP, expand=1, pady=5)

# Create a list to store the figure canvas for each subplot
fig = [0,0,0,0,0,0,0,0]
freq_labels = [0,0,0,0,0,0,0,0]
pw_labels = [0,0,0,0,0,0,0,0]
canvas = [0,0,0,0,0,0,0,0]
lines = [0,0,0,0,0,0,0,0]

# Add figure canvas for 8 subplots with adjusted height ratios
for i in range(8):
    new_frame = tk.Frame(master=frame)
    new_frame.pack(side='top')
    labels_frame = tk.Frame(master=new_frame)
    fig[i] = Figure(figsize=(8, 1))
    fig[i].subplots_adjust(hspace=2)
    fig[i].add_subplot()
    fig[i].axes[0].set_ylim(-0.2, 1.2)
    fig[i].axes[0].set_yticks([0, 1])
    fig[i].axes[0].set_yticklabels(['0', '1'])
    fig[i].axes[0].set_xlim(0,)
    fig[i].axes[0].set_ylabel("Channel {}".format(i+1), rotation=0, labelpad=30, fontsize=9, color="w", va="center",
                               fontweight="bold", fontstyle="italic", backgroundcolor="DarkBlue",
                                bbox=dict(boxstyle="round", fc="darkblue", ec="k", alpha=0.8))
    fig[i].axes[0].set_facecolor("black")
    fig[i].axes[0].set_xlabel("Time (us)")
    fig[i].axes[0].grid(True, color="white", lw=0.3)
    fig[i].axes[0].set_axisbelow(True)
    
    
    # Add a line to each subplot
    lines[i] = fig[i].axes[0].step([],[],lw=1, color="yellow")[0]
    
    # Add the figure canvas to the GUI
    canvas[i] = FigureCanvasTkAgg(fig[i], master=new_frame)
    canvas[i].get_tk_widget().pack(side=customtkinter.LEFT, fill=customtkinter.BOTH, expand=1,padx=5)
    labels_frame.pack(side='left',padx=5)
    freq_labels[i] = tk.Label(master=labels_frame, text='Avg Freq. = ', font=("Courier", 10), fg="green")
    freq_labels[i].pack(side='top',padx=5)
    pw_labels[i] = tk.Label(master=labels_frame, text='Pulse Width = ', font=("Courier", 10), fg="green")
    pw_labels[i].pack(side='top',padx=5)
    canvas[i].draw()


# Create a serial connection to read data from the logic analyzer
ser = serial.Serial(port='COM5', baudrate=9600)

# Create a list to store the data for each channel
data = [[] for _ in range(8)]
time_stamp = [0]*samples


# Function to establish the serial connection
def connect_serial():
    ser.open()
    print("Serial connection established.")


# Function to read and interpret n samples
def read_samples():
    global data
    global time_stamp
    global runningflag
    global samples

    while(runningflag):
        for _ in range(samples):
            # Read the sample message
            sample = ser.read(7)

            # Check if the sample is valid
            if len(sample) == 7 and sample[0] == ord('@') and sample[-1] == ord(';'):
                # Extract the channel logic levels and time stamp
                logic_levels = sample[1]

                # Convert the logic levels to binary representation
                logic_levels_bin = bin(logic_levels)[2:].zfill(8)
                time_stamp[_] = int.from_bytes(sample[2:6], byteorder='big')

                # Update the data for each channel
                for i in range(8):
                    data[i].append(int(logic_levels_bin[7-i]))
            else:
                if len(sample) != 7:
                    print("len Invalid sample received.")
                elif sample[0] != ord('@'):
                    print("@ Invalid sample received.")
                elif sample[-1] != ord(';'):
                    print("; Invalid sample received.")
                else:
                    print("Invalid sample received.")
                
        update_plots()


# Function to update the plots and calculate pulse width and average frequency
def update_plots():
    global data
    global time_stamp
    global samples
    global new_samples

    # Update the data for each subplot
    for i in range(8):
        lines[i].set_ydata(data[i])
        lines[i].set_xdata(time_stamp)
        fig[i].axes[0].set_xlim(0, time_stamp[-1])      
        canvas[i].draw()
        frame.update_idletasks()
        
        # Calculate pulse width
        pulse_width = [0]*8
        frequency = [0]*8

        for i in range(8):
            time1 = 0
            time2 = 0
            time3 = 0
            fr_counter = 0
            pw_counter = 0
            for j in range(samples-1):
                if data[i][j] == 0 and data[i][j+1] == 1:
                    time1 = time_stamp[j+1]
                    for k in range(j+1, samples-1):
                        if data[i][k] == 0 and data[i][k+1] == 1:
                            time2 = time_stamp[k+1]
                            frequency[i] += (10**6)/(time2 - time1)
                            fr_counter += 1
                            break
                    for k in range(j+1, samples-1):
                        if data[i][k] == 1 and data[i][k+1] == 0:
                            time3 = time_stamp[k+1]
                            pulse_width[i] += time3 - time1
                            pw_counter += 1
                            break
            frequency[i] = frequency[i]/fr_counter if fr_counter > 0 else 0
            pulse_width[i] = pulse_width[i]/pw_counter if pw_counter > 0 else 0

    # update pulse width and average frequency values for each channel in their drawn textbox
    for i in range(8):
        freq_labels[i].configure(text= "Avg Freq. = {:06} Hz".format((int)(frequency[i])))
        pw_labels[i].configure(text= "Pulse Width = {:06} us".format((int)(pulse_width[i])))
    
    ser.write(new_samples.to_bytes(1, byteorder='big', signed=False))    
    samples = new_samples
    time_stamp = [0]*samples
    data = [[] for _ in range(8)]
    
# create thread to read data from the logic analyzer
thread = threading.Thread(target=read_samples)
thread.start()

# Start the main event loop
frame.mainloop()
runningflag = False
