# Full 8-bit Logic Analyzer

## Overview
This project presents a Full 8-bit Logic Analyzer with USB interfacing, utilizing an AVR ATMega32 microcontroller and a Python-based GUI application using Custom Tkinter. Designed to capture and analyze digital signals in real-time, this tool is essential for electronics enthusiasts and professionals who require detailed signal analysis.

## Features
- **Real-Time Signal Plotting:** Captures digital signals from a signal generator and plots them on a Python GUI application.
- **Microcontroller-Based Sampling:** Utilizes the AVR ATMega32's PORT A to input 8 digital signals, which are then analyzed and timestamped.
- **Serial Communication:** Employs UART for data transmission between the microcontroller and the PC.
- **Dynamic Sampling Rate:** The GUI application can adjust the sampling rate, providing two-way communication with the hardware for time scaling.
- **Frequency and Pulse Width Calculations:** Ensures accurate representation of signals by calculating average frequency and pulse width.

## How It Works
1. A Signal Generator outputs a square wave to 8 channels.
2. The microcontroller continuously receives this output through PORT A.
3. It monitors, samples, and timestamps the signal states.
4. Data is sent to the PC via a Serial UART Module (USB to TTL).
5. The USB to TTL converter adjusts logic levels for USB compatibility.
6. The PC application plots the signals and calculates relevant metrics.

<p align="center">
  <img width="600" height="300" src="https://picsum.photos/460/300](https://github.com/Youssef-Global/Full_8-bit_Logic_Analyzer_based_on_ATMega32/assets/105471669/5bb8c128-0294-4782-b440-53bd01b6d0f5">
</p>
