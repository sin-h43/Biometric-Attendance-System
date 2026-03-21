/*
 * Fingerprint Enrollment Sketch
 * Run this ONCE to enroll each student's fingerprint.
 * After enrollment, upload the main attendance_system.ino
 *
 * Wiring same as main sketch:
 *   R307S TX --> GPIO 16 (RX2)
 *   R307S RX --> GPIO 17 (TX2)
 */

#include <Adafruit_Fingerprint.h>

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  if (!finger.verifyPassword()) {
    Serial.println("ERROR: Fingerprint sensor not found! Check wiring.");
    while (true) delay(1000);
  }

  Serial.println("Fingerprint sensor found!");
  Serial.println("=================================");
  Serial.println("Enter an ID (1-127) to enroll: ");
}

void loop() {
  if (Serial.available()) {
    int id = Serial.parseInt();
    if (id < 1 || id > 127) {
      Serial.println("Invalid ID. Use 1 to 127.");
      return;
    }

    Serial.printf("\nEnrolling ID #%d ...\n", id);
    enrollFingerprint(id);
  }
}

// ── Enroll a fingerprint with two scans ───────────────────────────────────
void enrollFingerprint(uint8_t id) {
  // ── First scan ──
  Serial.println("Place finger on sensor...");
  while (finger.getImage() != FINGERPRINT_OK);
  Serial.println("Image captured.");

  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("ERROR: Could not process image.");
    return;
  }

  Serial.println("Remove finger.");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  // ── Second scan ──
  Serial.println("Place same finger again...");
  while (finger.getImage() != FINGERPRINT_OK);
  Serial.println("Image captured.");

  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("ERROR: Could not process image.");
    return;
  }

  // ── Create model and store ──
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("ERROR: Fingerprints did not match. Try again.");
    return;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {
    Serial.println("ERROR: Could not store fingerprint.");
    return;
  }

  Serial.printf("SUCCESS: Fingerprint enrolled as ID #%d\n", id);
  Serial.println("=================================");
  Serial.println("Enter next ID to enroll, or upload main sketch.");
}
