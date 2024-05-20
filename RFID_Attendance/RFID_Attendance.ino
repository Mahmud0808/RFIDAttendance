// RFID-----------------------------
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

// NodeMCU--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// WiFi-----------------------------
#include <WiFiClient.h>

// Servo----------------------------
#include <Servo.h>

// RC522 RFID connections  NodeMCU ESP8266
#define SCK 14   // D5
#define MISO 12  // D6
#define MOSI 13  // D7
#define CS 15    // D8 RFID-RC522 SDA pin
#define RST 0    // D3
// ESP8266 GND to RC522 GND
// ESP8266 3.3V to RC522 3.3V (the power pin on the RC522 is labeled 3.3V)

// Servo pin conntections
const int trigPin = 5;  // D1
const int echoPin = 4;  // D2

// Buzzer pin connection
const unsigned char active_buzzer = 16;

MFRC522DriverPinSimple ss_pin(15);                                                 // Create pin driver. See typical pin layout above.
SPIClass &spiClass = SPI;                                                          // Alternative SPI e.g. SPI2 or from library e.g. softwarespi.
const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);  // May have to be set if hardware is not fully compatible to Arduino specifications.
MFRC522DriverSPI driver{
  ss_pin,
  spiClass,
  spiSettings
};  // Create SPI driver.

MFRC522 mfrc522{
  driver
};                      // Create MFRC522 instance.
WiFiClient wifiClient;  // Create WiFi instance.
Servo servo;            // Create Servo instance.

/* Set these to your desired credentials. */
const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPassword";
const char *device_token = "YourDeviceToken";

// Servo variables
long duration;
int distance;
int rotation;
bool openGate = false;
int openGateRotation = 150;
int closeGateRotation = 60;

// Ultrasonic variables
int distanceToCloseGate = 8;

// Server variables
String URL = "http://localhost/rfidattendance/getdata.php";  // URL for fetching RFID attendance data
// If the above URL doesn't work, open the command prompt and run 'ipconfig'.
// Find and copy the IPv4 Address of local network and replace 'localhost' with this address in the URL.
// Make sure to connect both XAMPP and NodeMCU to the same WiFi network.
String getData, Link;

// Card variables
String OldCardID = "null";
unsigned long previousMillis = 0;
int cooldownTime = 15000;

void setup() {
  delay(1000);
  setup_ultrasonic();
  setup_buzzer();
  Serial.begin(115200);
  setup_rfid();
  connectToWiFi();
}

void setup_ultrasonic() {
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input

  servo.attach(2, 500, 2500);  // D4, min rotation, max rotation
  servo.write(0);
  servo.write(closeGateRotation);
  rotation = closeGateRotation;
}

void setup_buzzer() {
  pinMode(active_buzzer, OUTPUT);
}

void setup_rfid() {
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
}

void loop() {
  if (!WiFi.isConnected()) {
    connectToWiFi();
  }

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  distance = duration * 0.034 / 2;

  // Prints the distance on the Serial Monitor
  // Serial.print("Distance: ");
  // Serial.println(distance);

  if (openGate) {
    if (distance <= distanceToCloseGate && rotation != closeGateRotation) {
      close_gate();
      openGate = false;
    } else if (rotation != openGateRotation) {
      open_gate();
    }
  }

  if (millis() - previousMillis >= cooldownTime) {
    previousMillis = millis();
    OldCardID = "null";
  }
  delay(50);

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  String CardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }

  if (CardID == OldCardID) {
    return;
  } else {
    OldCardID = CardID;
  }

  SendCardID(CardID);
  delay(1000);
}

void SendCardID(String Card_uid) {
  if (WiFi.isConnected()) {
    Serial.println("");
    Serial.println("Sending the Card ID");

    HTTPClient http;
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token);
    Link = URL + getData;
    http.begin(wifiClient, Link);

    int httpCode = http.GET();          // HTTP return code
    String payload = http.getString();  // Response payload
    if (payload == "") {
      payload = "none";
    }

    Serial.print("HTTP Code: ");
    Serial.println(httpCode);
    Serial.print("Card ID: ");
    Serial.println(Card_uid);
    Serial.print("Response: ");
    Serial.println(payload);

    if (httpCode == 200) {
      if (payload.substring(0, 5) == "login") {
        // String user_name = payload.substring(5);
        openGate = true;
      } else if (payload.substring(0, 6) == "logout") {
        // String user_name = payload.substring(6);
        openGate = true;
      } else if (payload == "successful") {
        openGate = false;
      } else if (payload == "available") {
        openGate = false;
      }

      beep_once();

      http.end();
    } else {
      beep_twice();

      if (httpCode == -1) {
        Serial.println("Error: Incompatible WiFi network. Could not connect to server.");
      }
    }
  } else {
    beep_twice();

    Serial.println("Error: WiFi not connected");
  }
}

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Attempting to connect to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_OFF);  // Ensure WiFi is turned off before connecting to prevent re-connection issues
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {  // Limit the number of connection attempts
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected successfully");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Error: Failed to connect to WiFi. Check credentials or network availability.");
  }
}

void open_gate() {
  servo.write(openGateRotation);
  rotation = openGateRotation;
  delay(1000);
}

void close_gate() {
  servo.write(closeGateRotation);
  rotation = closeGateRotation;
  delay(1000);
}

void beep_once() {
  digitalWrite(active_buzzer, HIGH);
  delay(100);
  digitalWrite(active_buzzer, LOW);
}

void beep_twice() {
  beep_once();
  delay(100);
  beep_once();
}
