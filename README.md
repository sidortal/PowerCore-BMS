# ⚡ PowerCore BMS

[![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-blue?logo=arduino)](https://www.arduino.cc/)  
[![Platform](https://img.shields.io/badge/Platform-ESP32-orange?logo=espressif)](https://www.espressif.com/)  
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)  

An ESP32-powered **Battery Management System (BMS) prototype** for **EV and energy storage safety monitoring**.  
It integrates **temperature, gas, and current sensors** with a multi-screen OLED dashboard, relay safety logic, and buzzer alerts.  
Developed as an academic + practical hardware project.

---

## 🎥 Demo Videos
- [▶️ Watch Demo Video 1](https://drive.google.com/file/d/VIDEO_ID_1/view?usp=sharing)  
- [▶️ Watch Demo Video 2](https://drive.google.com/file/d/VIDEO_ID_2/view?usp=sharing)

*(Hosted on Google Drive — click links to watch)*

---

## 🖼️ Project Photos
<p align="center">
  <img src="https://drive.google.com/uc?export=view&id=PHOTO_ID_1" width="350">
  <img src="https://drive.google.com/uc?export=view&id=PHOTO_ID_2" width="350">
  <img src="https://drive.google.com/uc?export=view&id=PHOTO_ID_3" width="350">
</p>

---

## 📑 Project Presentation
- [📂 View Presentation (PPT)](https://drive.google.com/file/d/PPT_ID/view?usp=sharing)

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
`10 September 2025`
