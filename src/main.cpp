#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL2qcnDkRjw"
#define BLYNK_TEMPLATE_NAME "Pet_feeder"
#define BLYNK_AUTH_TOKEN "tAJkzD4n9Dm2U_14t5AZhAPjkmnBfpOq"

#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <BlynkSimpleEsp32.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

/* Wi-Fi credentials */ 
#define WIFI_SSID "SETUP-1C17"
#define WIFI_PASSWORD "clever0316chose"

/* Firebase API */ 
#define API_KEY "AIzaSyCzYBIe06MhtwSoZw6g7cCqLdrzfx-ipoc"

/* Firebase Realtime DB */
#define DATABASE_URL "https://testdb-18eda-default-rtdb.firebaseio.com/"

/* Authentication */
#define USER_EMAIL "hongje1@uci.edu"
#define USER_PASSWORD "Ghdwjddms95!"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

Servo myServo;

String firebaseTime = "";
bool servoMoved = false;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);

  myServo.attach(15);

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
}

void sendToFirebase(String path, String value) {
  if (Firebase.RTDB.setString(&fbdo, path, value)) {
    Serial.println("Data sent to Firebase successfully");
  } else {
    Serial.println("Failed to send data to Firebase");
    Serial.println(fbdo.errorReason());
  }
}

void checkFirebaseTime() {
  if (Firebase.RTDB.getString(&fbdo, "/sensor/value")) {
    firebaseTime = fbdo.stringData();
    Serial.println(firebaseTime);
    
    timeClient.update();
    String currentTime = timeClient.getFormattedTime();
    Serial.println(currentTime);

    if (currentTime == firebaseTime && !servoMoved) {
      myServo.write(180); // Move servo to 180 degrees
      servoMoved = true;
      Serial.println("Servo moved at specified time");
    } else if (currentTime != firebaseTime) {
      servoMoved = false; // Reset the flag when the time changes
    }
  } else {
    Serial.println("Failed to get time from Firebase");
    Serial.println(fbdo.errorReason());
  }
}

BLYNK_WRITE(V1) {
  int buttonState = param.asInt();
  Blynk.virtualWrite(V2, 0); 
  if (buttonState == 1) {         // Button pressed
    myServo.write(180);
  } else if (buttonState == 0) {
    myServo.write(0);             // Reset servo
  }
}

BLYNK_WRITE(V3) {
  String sensorValue = param.asString();
  sendToFirebase("/sensor/value", sensorValue);
}

void loop() {
  Blynk.run();
  checkFirebaseTime();
  delay(1000);
}


