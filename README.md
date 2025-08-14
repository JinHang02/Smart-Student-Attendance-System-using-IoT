# Smart Student Attendance System using IoT 🎓📡

## ​ Demo

Watch the full demonstration here:  
👉 [Smart Attendance System Demo](https://www.youtube.com/watch?v=eFRZSLBrb7o)

---

## ​ Project Overview

This project is an automated attendance system leveraging:

- **RFID & Mbed OS**: Students scan RFID tags to log attendance.
- **ESP8266 Wi-Fi Module**: Handles cloud communication.
- **Google Sheets Integration**: Attendance data is sent via Google Apps Script and logged instantly.
- **User Feedback Interface**:
  - 16×2 I²C LCD displays prompts and student info.
  - LED indicators and buzzer alerts provide clear status confirmation.

---

## ​​ Features

| Feature               | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| **Real-time logging**| Sends attendance data immediately to Google Sheets using HTTPS POST.        |
| **User interaction** | Displays “Tap your card,” shows student name/ID, and provides audio/visual feedback. |
| **Error handling**   | Unknown IDs trigger a warning display and buzzer pattern for quick retry.   |

---

## ​ Hardware Components

- STM32 microcontroller (Mbed-compatible board)
- MFRC522 RFID reader
- ESP8266 Wi-Fi module
- 16×2 I²C LCD (PCF8574 adapter)
- LEDs for Wi-Fi status
- Buzzer for sound alerts
- Passive RFID cards

---

## ​ Software Stack

- **Mbed OS** – for microcontroller firmware
- **MFRC522 library** – RFID interaction
- **ESP8266 AT commands** – for Wi-Fi and SSL communication
- **Google Apps Script** – hosted on Google Drive to receive and log JSON data

---
## ​ Future Enhancements

- Auto-register new RFID tags with user-provided names/IDs.
- Add timestamp display on the LCD at scan time.
- Enhance security with HTTPS certificate validation or OAuth.
- Build a web dashboard for attendance analytics.


## ​ License

This project is licensed under the MIT License. See `LICENSE` for details.

