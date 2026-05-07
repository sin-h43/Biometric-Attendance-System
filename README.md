# Biometric Attendance System

An IoT-based biometric attendance system built using **ESP32**, **AS608 Fingerprint Sensor**, **IR Sensor**, and **Google Sheets** for real-time attendance logging.

## Overview

This project automates attendance marking using fingerprint authentication and presence detection. The ESP32 reads fingerprint data, verifies the user, checks physical presence using an IR sensor, and sends attendance records to Google Sheets through a Google Apps Script web app.

## Features

- Fingerprint-based user authentication
- IR sensor-based presence detection
- Real-time attendance logging to Google Sheets
- ESP32 Wi-Fi connectivity
- HTTP-based data transfer
- Separate fingerprint enrollment code
- Lightweight and low-cost IoT implementation

## Tech Stack

- ESP32
- Arduino IDE
- AS608 Fingerprint Sensor
- IR Sensor
- Google Apps Script
- Google Sheets
- C/C++

## Repository Structure

```text
Biometric-Attendance-System/
│
├── ESP32.ino
├── Arduino.ino
├── enroll_fingerprint.ino
├── google_apps_script.gs
└── README.md
```
## Working
The fingerprint sensor captures the user's fingerprint.
ESP32 verifies the fingerprint ID.
The IR sensor checks whether the person is physically present.
If authentication is successful, ESP32 sends data to Google Apps Script.
Google Apps Script updates the attendance record in Google Sheets.

## Hardware Requirements
ESP32 development board
AS608 fingerprint sensor
IR sensor
Jumper wires
Breadboard
USB cable
Wi-Fi connection (2.4Hz)
Software Requirements
Arduino IDE
ESP32 board package
Adafruit Fingerprint Sensor Library
Google Sheets
Google Apps Script

Setup Instructions
1. Clone the Repository
git clone https://github.com/sin-h43/Biometric-Attendance-System.git
cd Biometric-Attendance-System
2. Install Required Libraries
In Arduino IDE, install:
Adafruit Fingerprint Sensor Library
WiFi library
HTTPClient library
3. Upload Fingerprint Enrollment Code
Open enroll_fingerprint.ino in Arduino IDE and upload it to ESP32.
Use the serial monitor to enroll fingerprints and assign fingerprint IDs.

4. Set Up Google Sheets
Create a Google Sheet with columns such as:
Name | Fingerprint ID | Date | Time | Status
5. Deploy Google Apps Script
Open google_apps_script.gs, paste it into Google Apps Script, and deploy it as a web app.
Set access permission to allow requests from ESP32.
Copy the generated web app URL.

6. Update ESP32 Code
In ESP32.ino, update:
   const char* ssid = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   String serverName = "YOUR_GOOGLE_APPS_SCRIPT_WEB_APP_URL";
7. Upload Main Code
Upload ESP32.ino to the ESP32 board.
Open the Serial Monitor to check fingerprint detection and attendance logging.

## Applications
 Classroom attendance
 Lab attendance
 Office entry logging
 Small organization attendance management
 IoT-based access tracking

## Skills Demonstrated
 Embedded systems programming
 IoT communication
 Sensor integration
 Fingerprint authentication
 Google Sheets automation
 HTTP requests using ESP32
 Real-time data logging
