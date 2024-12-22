#include <ESP8266WiFi.h>
#include <espnow.h>

// MAC address of the Motor ESP (update with correct MAC address)
uint8_t motorESPAddress[] = {0xEC, 0x64, 0xC9, 0xCE, 0x06, 0x3D};//EC:64:C9:CE:06:3D  ec:64:c9:ce:06:3d

const int relayPin1 = D1;  // Pin connected to the relay controlling the solenoid valve
const int relayPin2 = D2;  // Pin connected to the relay controlling the solenoid valve
int desiredMoistureLevel = 65;

typedef struct {
  int rollNo;
  int moistureValue;
} SensorData;

SensorData sensor1Data, sensor2Data;
bool sensor1Received = false, sensor2Received = false;

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  SensorData receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  if (receivedData.rollNo == 1) {
    sensor1Data = receivedData;
    sensor1Received = true;
    Serial.print("Head received from Sensor 1 - Moisture: ");
    Serial.println(receivedData.moistureValue);
  } else if (receivedData.rollNo == 2) {
    sensor2Data = receivedData;
    sensor2Received = true;
    Serial.print("Head received from Sensor 2 - Moisture: ");
    Serial.println(receivedData.moistureValue);
  }

  if (sensor1Received && sensor2Received) {
    int averageMoistureValue = (sensor1Data.moistureValue + sensor2Data.moistureValue) / 2;

    SensorData avgData;
    avgData.rollNo = 6;  //Head 1-3. Head 2-6
    avgData.moistureValue = averageMoistureValue;

    esp_now_send(motorESPAddress, (uint8_t *)&avgData, sizeof(avgData));
    Serial.print("Head sent average moisture: ");
    Serial.println(averageMoistureValue);

    if (averageMoistureValue < desiredMoistureLevel) {
      digitalWrite(relayPin1, LOW);  // Open valve
      digitalWrite(relayPin2, LOW);  // Open valve
      Serial.println("Valve opened (moisture below threshold)");
    } else {
      digitalWrite(relayPin1, HIGH);  // Close valve
      digitalWrite(relayPin2, HIGH);  // Close valve
      Serial.println("Valve closed (moisture above threshold)");
    }

    sensor1Received = false;
    sensor2Received = false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, HIGH);  // Ensure the relay is initially off
  digitalWrite(relayPin2, HIGH);  // Ensure the relay is initially off

  WiFi.mode(WIFI_AP_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed on Head ESP");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(motorESPAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_register_recv_cb(onDataRecv);
  Serial.println("ESP-NOW initialized on Head ESP");
}

void loop() {
  // Head ESP listens for incoming data
}