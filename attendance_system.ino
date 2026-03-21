/*
 * Biometric Attendance System
 * Hardware: ESP32 + R307S Fingerprint Sensor + 16x2 LCD (I2C)
 * Database: Google Sheets via Apps Script Web App
 *
 * Wiring:
 *   R307S TX  --> ESP32 GPIO 16 (RX2)
 *   R307S RX  --> ESP32 GPIO 17 (TX2)
 *   R307S VCC --> 3.3V
 *   R307S GND --> GND
 *   LCD SDA   --> ESP32 GPIO 21
 *   LCD SCL   --> ESP32 GPIO 22
 *   LCD VCC   --> 5V
 *   LCD GND   --> GND
 *
 * Libraries needed (install via Arduino Library Manager):
 *   - Adafruit Fingerprint Sensor Library
 *   - LiquidCrystal I2C  (by Frank de Brabander)
 *   - WiFi (built-in ESP32)
 *   - HTTPClient (built-in ESP32)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>

// ─── CONFIG: Edit these ────────────────────────────────────────────────────
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Paste your Google Apps Script Web App URL here (see google_apps_script.gs)
const char* SHEETS_URL = "https://script.google.com/macros/s/YOUR_SCRIPT_ID/exec";

// Map fingerprint IDs to student names (add as many as needed)
// ID must match the ID used during enrollment
struct Student {
  uint8_t id;
  const char* name;
  const char* rollNo;
};

Student students[] = {
  {1, "Alice Johnson",  "CS001"},
  {2, "Bob Smith",      "CS002"},
  {3, "Carol White",    "CS003"},
  // Add more students here...
};

const int STUDENT_COUNT = sizeof(students) / sizeof(students[0]);
// ──────────────────────────────────────────────────────────────────────────

// LCD: address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Fingerprint sensor on UART2 (GPIO 16=RX, 17=TX)
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

// ── Setup ──────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Init LCD
  lcd.init();
  lcd.backlight();
  lcdPrint("Attendance Sys", "Booting...");

  // Init fingerprint sensor
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    lcdPrint("Sensor Error!", "Check wiring");
    Serial.println("Fingerprint sensor not found!");
    while (true) delay(1000);
  }

  Serial.println("Fingerprint sensor OK");
  finger.getTemplateCount();
  Serial.printf("Stored templates: %d\n", finger.templateCount);

  // Connect to WiFi
  lcdPrint("Connecting WiFi", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
    lcdPrint("WiFi Connected!", WiFi.localIP().toString().c_str());
  } else {
    lcdPrint("WiFi Failed!", "Offline mode");
    Serial.println("\nWiFi connection failed. Running offline.");
  }

  delay(2000);
  lcdPrint("Place Finger", "to Mark Present");
}

// ── Main Loop ──────────────────────────────────────────────────────────────
void loop() {
  int fingerprintID = getFingerprintID();

  if (fingerprintID > 0) {
    // Find student by ID
    Student* student = findStudent(fingerprintID);

    if (student != NULL) {
      Serial.printf("Recognized: %s (%s)\n", student->name, student->rollNo);
      lcdPrint(student->name, "Sending...");

      // Send to Google Sheets
      if (WiFi.status() == WL_CONNECTED) {
        bool success = sendToSheets(student->name, student->rollNo, fingerprintID);
        if (success) {
          lcdPrint("Present!", student->rollNo);
        } else {
          lcdPrint("Upload Failed", student->rollNo);
        }
      } else {
        lcdPrint("Present! (Offline)", student->rollNo);
      }

    } else {
      // ID recognized by sensor but not in our student list
      Serial.printf("Unknown ID: %d\n", fingerprintID);
      lcdPrint("Unknown ID:", String(fingerprintID).c_str());
    }

    delay(3000);
    lcdPrint("Place Finger", "to Mark Present");
  }

  delay(100);
}

// ── Get Fingerprint ID ─────────────────────────────────────────────────────
// Returns matched ID (1–127) or -1 if no match / error
int getFingerprintID() {
  uint8_t result = finger.getImage();

  if (result == FINGERPRINT_NOFINGER)   return -1;
  if (result != FINGERPRINT_OK)         return -1;

  result = finger.image2Tz();
  if (result != FINGERPRINT_OK)         return -1;

  result = finger.fingerFastSearch();
  if (result != FINGERPRINT_OK) {
    lcdPrint("No Match!", "Try Again...");
    delay(2000);
    lcdPrint("Place Finger", "to Mark Present");
    return -1;
  }

  // finger.fingerID = matched ID, finger.confidence = match score
  Serial.printf("ID: %d  Confidence: %d\n", finger.fingerID, finger.confidence);
  return finger.fingerID;
}

// ── Find Student by Fingerprint ID ────────────────────────────────────────
Student* findStudent(uint8_t id) {
  for (int i = 0; i < STUDENT_COUNT; i++) {
    if (students[i].id == id) return &students[i];
  }
  return NULL;
}

// ── Send Attendance to Google Sheets ──────────────────────────────────────
bool sendToSheets(const char* name, const char* rollNo, int fpID) {
  HTTPClient http;

  // Build URL with query parameters
  String url = String(SHEETS_URL)
    + "?name="   + urlEncode(name)
    + "&roll="   + urlEncode(rollNo)
    + "&fpid="   + String(fpID);

  Serial.println("Sending: " + url);

  http.begin(url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // follow Google's redirect
  int httpCode = http.GET();

  Serial.printf("HTTP Response: %d\n", httpCode);

  String response = http.getString();
  Serial.println("Response: " + response);
  http.end();

  // HTTP 200 = success
  return (httpCode == 200);
}

// ── LCD Helper ─────────────────────────────────────────────────────────────
void lcdPrint(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// ── URL Encode (spaces → %20, basic) ──────────────────────────────────────
String urlEncode(const char* str) {
  String encoded = "";
  while (*str) {
    char c = *str++;
    if (c == ' ')       encoded += "%20";
    else if (c == '&')  encoded += "%26";
    else if (c == '=')  encoded += "%3D";
    else                encoded += c;
  }
  return encoded;
}
