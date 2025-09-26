# ⚡ PowerCore BMS

[![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-blue?logo=arduino)](https://www.arduino.cc/)  
[![Platform](https://img.shields.io/badge/Platform-ESP32-orange?logo=espressif)](https://www.espressif.com/)  
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)  

An ESP32-powered **Battery Management System (BMS) prototype** for **EV and energy storage safety monitoring**.  
It integrates **temperature, gas, and current sensors** with a multi-screen OLED dashboard, relay safety logic, and buzzer alerts.  
Developed as an academic + practical hardware project.

---

## 🎥 Demo Videos
- [▶️ Watch Demo Video 1](https://drive.google.com/file/d/1IXptzPIsLpXgdGexhmpmoqSfk5SGxDSW/view?usp=drive_link)  
- [▶️ Watch Demo Video 2](https://drive.google.com/file/d/1eXrK253PXBS8xrBsJ-vRTsRqrHc911hD/view?usp=drive_link)

*(Hosted on Google Drive — click links to watch)*

---

## 🖼️ Project Photos
<p align="center">
  <img src="https://drive.google.com/file/d/1SEXGgC7r_PIKoTYpvXCxfrO7-Enog7RC/view?usp=drive_link" width="350">
  <img src="https://drive.google.com/file/d/1EKDHQS_w_Zt0m6lvbIdf2jfPhjnDum8q/view?usp=drive_link" width="350">
  <img src="https://drive.google.com/file/d/1lbuxYLbKJwxDPGmoq-CzddW8A3yhKGf1/view?usp=drive_link" width="350">
</p>

---

## 📑 Project Presentation
- [📂 View Presentation (PPT)](https://docs.google.com/presentation/d/1bvNAJ2W0TkVCBMyxPtJjyX1ubfwUfUWg/edit?usp=drive_link&ouid=102921229719269680219&rtpof=true&sd=true)

---

## 🚀 Features
- **Sensor Integration**
  - MLX90614 → Pack & ambient temperature  
  - NTC thermistor → Surface temperature  
  - INA219 → Current + simulated SOH  
  - Gas sensor → Fume/PPM detection  

- **Smart Control**
  - Dual relay logic (temperature + gas thresholds)  
  - Safety cut-offs for overheating (>40 °C) and hazardous fumes (>4000 PPM)  
  - Auto-reset when safe conditions return  

- **User Interface**
  - OLED with 4 rotating screens:  
    1. Temperatures + Gas levels  
    2. Pack voltage, current, SOH  
    3. Individual cell voltages with progress bars  
    4. Restored per-cell capacity stats  
  - Safety alerts → buzzer + on-screen warning  
  - Movie-style scrolling intro + boot beep  

- **Visual Indicators**
  - Multi-color LEDs → temperature ranges  
  - Gas detection → LED alerts  

---

## 📂 Repo Structure

---

## ⚡ Applications
- EV battery safety prototyping  
- Energy storage monitoring systems  
- Academic & research BMS projects  

---

## 👨‍💻 Authors
- Siddhesh Dangade  
- Swapnil Naikawadi  
- Sushant Dalvi  
- Kishan Kumar  
- Shreyas Kundapur  
- Rushikesh Mane  

Guided by:  
- **Prof. Sahil Goyal**  
- **Prof. Uday Apte**  

---

## 📅 Project Date
` 09/09/2025`
