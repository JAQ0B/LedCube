# 🔲 LED Cube – ESP32-Controlled Web Interface Project

![LED Cube Photo](assets/ledcube.jpg)

> A personal hardware project where I built a 4x4x4 LED Cube powered by an ESP32, featuring a custom web interface to control animations in real time.

---

## ✨ About the Project

This was a beginner-level project I created to improve my soldering and hardware skills. I wanted to make something visually interesting, so I designed and built a 4x4x4 LED Cube and added a **Wi-Fi controlled animation system** using an ESP32 microcontroller.

Rather than hardcoding animation sequences or using physical buttons, the cube runs its own **web interface**. Once connected to Wi-Fi, the ESP32 prints its IP address in the serial monitor. Users can access the cube’s control panel through any browser on the same network to **select and trigger animations**.

Some animations worked well, while others were harder to make visually effective due to the small size of the cube — but this was part of the learning process, and I’m proud of the result.

---

## ⚙️ Key Features

- 4x4x4 LED cube built from scratch and driven by shift registers
- Powered by an **ESP32** with integrated Wi-Fi
- Built-in **web interface** to control animations in real-time
- Modular animation logic with the ability to expand
- Great practice in soldering, basic electronics, and networking

---

## 🔒 Security Note

Wi-Fi credentials are stored in a separate `WifiCredentials.h` file, which is **excluded from version control** using `.gitignore`.  
Please do the same if you fork or clone this project.

---

## 🛑 Status

This project is **not actively maintained**, but I **welcome contributions** — especially improvements to existing animations or entirely new ones.

Feel free to fork the repo and show off your own animations!

---

## 📸 LED Cube Preview


![LED Cube Front View](/LedCube.jpg)
