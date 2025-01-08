# Frequency Detection System with ESP32

This project is a real-time frequency detection system using the ESP32 microcontroller and an INMP microphone. The system detects sound frequencies up to 10 kHz and provides results through a web interface.

## Features
- Real-time frequency detection with FFT analysis.
- Short (1-second) and long (4-second) FFT computations.
- Fire alarm trigger for specific frequency ranges (3000–3500 Hz).
- JSON-based API for frequency data.

## Components
- **ESP32 Microcontroller**: For computation and Wi-Fi connectivity.
- **INMP Microphone**: Captures audio signals.
- **Push Button**: Triggers FFT computations.
- **LED**: Indicates processing status.

## How It Works
1. The INMP microphone captures audio signals and sends them to the ESP32.
2. FFT is performed on the signals to detect the dominant frequencies.
3. Detected frequencies are hosted on an HTTP server, accessible via a browser.

## How to Use
1. **Upload the Code**:
   - Open `src/main.ino` in the Arduino IDE.
   - Configure your Wi-Fi credentials (`ssid` and `password`).
   - Upload the code to your ESP32.
2. **Connect to Wi-Fi**:
   - Ensure your ESP32 is connected to the same network.
   - Open the ESP32's IP address in a web browser.
3. **View Frequency Data**:
   - Access real-time frequency data in JSON format.

## Future Scope
- Improved frequency resolution.
- Enhanced noise filtering.
- Integration of machine learning for sound classification.
