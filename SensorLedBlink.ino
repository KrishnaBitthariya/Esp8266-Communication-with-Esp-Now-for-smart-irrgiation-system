#include <ESP8266WiFi.h>
#include <espnow.h>


// head 1 - 0xEC, 0x64, 0xC9, 0xCE, 0x01, 0x3E
// Head 2  - 0xEC, 0x64, 0xC9, 0xCE, 0x08, 0x7F
// ec:64:c9:c9:c3:17. 0xEC, 0x64, 0xC9, 0xC9, 0xC3, 0x17
uint8_t headESPAddress[] = {0xEC, 0x64, 0xC9, 0xCE, 0x01, 0x3E};  // Head 1
//uint8_t headESPAddress[] = {0xEC, 0x64, 0xC9, 0xCE, 0x08, 0x7F};  // head 2

const int ledPin = D2; // LED pin for blinking

// Structure to hold sensor data
typedef struct {
 int rollNo;
 int moistureValue;
} SensorData;


SensorData sensorData;  // Instance of the structure


void setup() {
 Serial.begin(115200);

  pinMode(ledPin, OUTPUT); // Set LED pin as output

 // Initialize WiFi in station mode
 WiFi.mode(WIFI_STA);


 // Initialize ESP-NOW
 if (esp_now_init() != 0) {
   Serial.println("ESP-NOW initialization failed");
   return;
 }


 // Set the ESP-NOW role if needed
 esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);


 Serial.println("Sensor ESP setup completed.");


 // Set the roll number for this sensor
 sensorData.rollNo = 2;  // Update this for each sensor
}


void loop() {
 // Simulating a moisture reading from sensor (analog read)
 int moistureValue = analogRead(A0);  // Assuming A0 is the analog pin for the moisture sensor
 Serial.print(moistureValue);

 sensorData.moistureValue = map(moistureValue, 600, 1023, 100, 0);  // Convert the raw value to percentage


 // Send the data to the Head ESP
 esp_now_send(headESPAddress, (uint8_t *)&sensorData, sizeof(sensorData));
  digitalWrite(ledPin, HIGH);
  delay(100);  // LED on for 100ms
  digitalWrite(ledPin, LOW);

 // Print for debugging
 Serial.print("Sent Roll No: ");
 Serial.print(sensorData.rollNo);
 Serial.print(" - Moisture Value: ");
 Serial.println(sensorData.moistureValue);


 // Wait for some time before sending again (e.g., 1 seconds)
 delay(1000);
}