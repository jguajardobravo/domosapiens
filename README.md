# 🏡 domosapiens – Smart Home Control System

**Design and implementation of a low-cost, scalable, and privacy-focused remote control system for home automation devices.**

This project was developed as a Bachelor's Thesis at the University of Zaragoza (EINA) and demonstrates how ESP microcontrollers can power custom IoT automation for home use—independent from cloud services and with full control over user data.

---

## 📌 Features

- 💡 **Aquarium Light Control**  
  Remotely toggle a 12V LED strip and a 5V lamp using a microcontroller-based server.

- 🌱 **Smart Garden Irrigation**  
  Independently activate two irrigation zones and run scheduled watering programs.

- 🌡️ **Weather Station**  
  Capture temperature and humidity using a DHT22 sensor and store logs on an SD card for graphical display.

---

## 🧠 Architecture Overview

The system is built on a **three-layer model**:

1. **Client Layer**  
   - Android mobile app (built in Java using Android Studio)  
   - Web interface (HTTP server)

2. **Frontend Layer**  
   - ESP32-based microcontroller managing requests via TCP/HTTP  
   - Delegates commands to backend devices

3. **Backend Layer**  
   - ESP32 and ESP8266-01S microcontrollers controlling the physical devices  
   - Each device functions as an independent TCP server

All components run on the same local network, allowing full offline operation. Remote control is available using Dynamic DNS (NOIP) and port forwarding, without depending on third-party platforms.

---

## ⚙️ Technologies Used

| Component             | Technology |
|----------------------|------------|
| Microcontrollers      | ESP32-WROOM-32D, ESP8266-01S |
| App Development       | Android Studio (Java) |
| Microcontroller IDE   | Arduino IDE (C++) |
| Circuit Design        | Fritzing |
| Architecture Modeling | ArchiMate |
| Flow Diagrams         | Draw.io |
| Data Storage          | SD Card |
| Communication         | TCP/IP, HTTP |
| DNS Access            | NOIP Dynamic DNS |

---

## 📦 Repository Structure

domosapiens/
├── AndroidStudio/ # Android app source code
├── Archi/ # Architecture diagrams (ArchiMate)
├── Arduino/ # Arduino sketches (ESP8266, ESP32)
├── Draw.io/ # Application logic diagrams
├── Fritzing/ # Electronic schematics
├── luz_terraza.ino # Standalone Arduino module
└── Memoria_TFG_...docx # Full project thesis (Spanish)

## 🚀 How to Run

1. Flash each Arduino sketch to its corresponding ESP board.
2. Set static IPs on your local router based on each board's MAC address.
3. Configure NAT (port forwarding) to expose the frontend if remote access is required.
4. Install the Android app or access the HTTP interface via a browser.

---

## 📚 Documentation

The full project documentation is available (in Spanish):  
📄 `Memoria_TFG_Diseño e implementación de un sistema para el control remoto de dispositivos domóticos.docx`

---

## 🌟 Future Enhancements

- Add scheduling logic to lighting modules.  
- Integrate soil moisture sensors for smarter irrigation.  
- Add camera-based monitoring using newer ESP32-CAM modules.  
- Improve mobile app UI and add user authentication.

---

## 👨‍💻 Author

**Javier Guajardo Bravo**  
Bachelor’s Degree in Electronics and Communications Engineering – University of Zaragoza

---

## 🛠 License

This repository is shared for educational and research purposes.
