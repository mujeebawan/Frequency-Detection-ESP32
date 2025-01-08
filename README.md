# Frequency Detection System with ESP32

This project implements a real-time frequency detection system using the ESP32 microcontroller and an INMP microphone. The system detects sound frequencies up to 10 kHz, performs FFT analysis, and serves the data via a JSON-based HTTP API.

---

## Features
- Real-time frequency detection with 1-second and 4-second FFT analysis.
- Fire alarm trigger for the 3000–3500 Hz frequency range.
- JSON API to display detected frequencies in a browser.

---

## Components
- **ESP32 Microcontroller**: For computation and Wi-Fi connectivity.
- **INMP Microphone**: Captures audio signals.
- **Push Button**: Triggers FFT computations.
- **LED**: Indicates processing status.
- **3.7V Battery with Boost Converter**: Powers the system.

---

## Hardware Connections
| **Component**        | **ESP32 Pin**         | **Description**                        |
|----------------------|-----------------------|----------------------------------------|
| INMP Microphone BCLK | GPIO33                | I2S Clock Pin                          |
| INMP Microphone LRCLK| GPIO25                | I2S Word Select (Left/Right) Pin       |
| INMP Microphone DOUT | GPIO32                | I2S Data Out Pin                       |
| Push Button          | GPIO35                | Triggers FFT computation on press      |
| LED                  | GPIO2                 | Indicates when FFT is being processed  |
| Power                | VIN (via Boost)       | 5V input from the boost converter      |

---

## Powering Up
- **3.7V Battery**: Connect the battery to a boost converter to step up to 5V.
- **ESP32 Power Supply**: Connect the 5V output of the boost converter to the VIN pin on the ESP32.
- Ensure proper grounding between all components.

---

## How to Use
1. **Setup Hardware**:
   - Follow the hardware connection table above to connect the components to the ESP32.
2. **Upload Code**:
   - Open `src/main.ino` in the Arduino IDE.
   - Update the `ssid` and `password` in the code with your Wi-Fi credentials.
   - Upload the code to the ESP32.
3. **Power the System**:
   - Power up the ESP32 and ensure the LED blinks briefly at startup.
4. **Access the Web Interface**:
   - Connect to the same Wi-Fi network as the ESP32.
   - Open the ESP32's IP address (printed in the Serial Monitor) in your web browser.
   - View the detected frequency data in JSON format.

---

## Results
- Successfully detects sound frequencies in the range of 0–10 kHz.
- Accurately triggers a "Fire Alarm" for frequencies between 3000–3500 Hz.
- Real-time frequency data is accessible via the HTTP server.

---

## Challenges & Solutions
- **Spectral Leakage**: Resolved by applying Hamming windowing during FFT.
- **Memory Constraints**: Optimized DMA buffer size and reduced FFT sample count.
- **Noise**: Improved clarity with a higher sampling frequency (40 kHz).

---

## Future Scope
- Improve frequency resolution with more FFT samples.
- Implement advanced noise filtering techniques.
- Add machine learning for sound classification.

---

## License
This project is licensed under the MIT License. See the `LICENSE` file for details.

## Example JSON Output
```json
{
  "shortFreq": 3150.25,
  "longFreq": 3200.12
}
