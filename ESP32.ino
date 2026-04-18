#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "PASSWORD";

// Replace with your Deployment URL from Google Apps Script
String serverName = "https://script.google.com/macros/s/AKfycbyLmDmcfZfWP6pwK8zxsJUUDUqVKMyNuHppjBBFv85zEeC1jAYg2P-sQtawi_HoN96Jaw/exec";

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  if (Serial2.available()) {
    String message = Serial2.readStringUntil('\n');
    message.trim();

    if (message == "STATUS:ALIVE") {
      Serial.println("Status: System Online");
    } 
    else if (message.startsWith("ID:")) {
      String idValue = message.substring(3);
      Serial.println("New ID received: " + idValue);
      sendDataToGoogleSheets(idValue);
    }
  }
}

void sendDataToGoogleSheets(String id) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Google Scripts require redirection, so we use this:
    String url = serverName + "?id=" + id;
    
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    int httpResponseCode = http.GET(); // Apps Script doPost can handle parameters via GET or POST
    
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println("Response: " + payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
