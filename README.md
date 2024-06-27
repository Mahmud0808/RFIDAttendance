## NodeMCU ESP8266 Smart Attendance System

This repository contains the code for a Smart Attendance System based on NodeMCU ESP8266. The system includes a web user interface for tracking attendance records, which is deployed using XAMPP. It also features an RFID reader for directly reading student ID cards and recording attendance in real-time. Additionally, a servo motor and ultrasonic sensor are used to open and close a gate, and a piezo buzzer indicates successful or declined readings.

### Features

- Web interface for tracking attendance records
- RFID reader for reading student ID cards
- Real-time attendance recording
- Servo motor and ultrasonic sensor for gate control
- Piezo buzzer for indicating successful or declined readings

### Setup Instructions

> Don't forget to check `Project Report.pdf`

- Clone this repository to your local machine.
- Set up XAMPP and deploy the web interface by placing the files in the  htdocs  folder.
- Connect the NodeMCU ESP8266 board to your computer and upload the code using the Arduino IDE.
- Connect the RFID reader, servo motor, ultrasonic sensor, and piezo buzzer to the appropriate GPIO pins on the NodeMCU board.
- Power on the system and start tracking attendance!

### Usage

Open the web interface to view attendance records.
Present a student ID card to the RFID reader to record attendance.
The gate will automatically open if the attendance is successfully recorded.
The piezo buzzer will sound to indicate a successful or declined reading.

Feel free to contribute to this project by forking and submitting a pull request. If you have any questions or suggestions, please open an issue.

Happy attendance tracking! 🎓📊🔐🔔
