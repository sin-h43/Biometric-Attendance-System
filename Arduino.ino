#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pins
const int IR_PIN = 4;

// Fingerprint sensor
SoftwareSerial mySerial(2, 3); // AS608
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Try 0x3F if 0x27 doesn't work

unsigned long lastPingTime = 0;
bool userDetected = false;

void showMessage(String line1, String line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print("                "); // clear line 2
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setup() {
  Serial.begin(9600);   // To ESP32
  pinMode(IR_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  showMessage("System Ready", "Initializing");

  finger.begin(57600);
  if (finger.verifyPassword()) {
    showMessage("System Ready", "Sensor Found");
  } else {
    showMessage("Sensor Error", "Check Wiring");
    while (1);
  }

  delay(2000);
  showMessage("Waiting for", "User");
}

void loop() {
  int irState = digitalRead(IR_PIN);

  // Heartbeat every 10s
  if (millis() - lastPingTime >= 10000) {
    lastPingTime = millis();
    Serial.println("STATUS:ALIVE");
  }

  // IR detects user
  if (irState == LOW) {
    if (!userDetected) {
      userDetected = true;
      showMessage("Finger Detected", "Place Finger");
      delay(500);
    }

    showMessage("Scanning...", "");

    int fingerID = getFingerprintID();

    if (fingerID > 0) {
      Serial.print("ID:");
      Serial.println(fingerID);

      showMessage("ID Found:", String(fingerID));
      delay(3000);
      showMessage("Waiting for", "User");
      userDetected = false;
    } else {
      showMessage("No Match Found", "Try Again");
      delay(1500);
      showMessage("Waiting for", "User");
      userDetected = false;
    }
  } else {
    if (userDetected) {
      userDetected = false;
      showMessage("Waiting for", "User");
    }
  }
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}
