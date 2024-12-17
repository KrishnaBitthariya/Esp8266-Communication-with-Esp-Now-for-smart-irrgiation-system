#include <ESP8266WiFi.h>
#include <espnow.h>


const int motorPin = D1;  // Pin connected to the motor control relay
int desiredMoistureLevel = 65;

// Battery monitoring variables
float batteryVoltage;
float batteryPercentage;

// Initial moisture levels
int head1Moisture = 100;
int head2Moisture = 100;
float head3Moisture = 100;

// Previous moisture values to track changes
int prevHead1Moisture = 100;
int prevHead2Moisture = 100;
float prevHead3Moisture = 100;

// MAC address of the target ESP
uint8_t targetMacAddress[] = {0xEC, 0x64, 0xC9, 0xC9, 0xC3,0xC17};  // MAC: 08:f9:e0:75:d8:30.  Replace with the actual MAC address

// Struct to receive data from sensors
typedef struct {
 int rollNo;
 int moistureValue;
} SensorData;

// Struct to send data with average moisture for each head and motor status
typedef struct {
  int avgMoistureHead1;
  int avgMoistureHead2;
  int avgMoistureHead3;
  bool motorStatus;
} MotorStatusData;

MotorStatusData motorStatusData;

// Function to calculate average moisture values (for each head individually)
void updateAverageMoistureValues() {
  motorStatusData.avgMoistureHead1 = head1Moisture;
  motorStatusData.avgMoistureHead2 = head2Moisture;
  motorStatusData.avgMoistureHead3 = head3Moisture;
}

// Callback function for receiving data
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  SensorData receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  bool updateMotorState = false;

  if (receivedData.rollNo == 3) {
    if (receivedData.moistureValue != head1Moisture) {
      prevHead1Moisture = head1Moisture;
      head1Moisture = receivedData.moistureValue;
      updateMotorState = true;
    }
    Serial.print("Motor ESP received average moisture from Head 1: ");
    Serial.println(head1Moisture);
  } else if (receivedData.rollNo == 6) {
    if (receivedData.moistureValue != head2Moisture) {
      prevHead2Moisture = head2Moisture;
      head2Moisture = receivedData.moistureValue;
      updateMotorState = true;
    }
    Serial.print("Motor ESP received average moisture from Head 2: ");
    Serial.println(head2Moisture);
  } else if (receivedData.rollNo == 9) {
    if (receivedData.moistureValue != head3Moisture) {
      prevHead3Moisture = head3Moisture;
      head3Moisture = receivedData.moistureValue;
      updateMotorState = true;
    }
    Serial.print("Motor ESP received average moisture from Head 3: ");
    Serial.println(head3Moisture);
       // Send the motor status data to the specified ESP
    
  }
     // Send the motor status data to the specified ESP
    

  // Update motor state only if any moisture value has changed
  if (updateMotorState) {
    // Update average moisture values for each head
   //updateAverageMoistureValues();

    // Determine motor status based on desired moisture level
    if (head1Moisture < desiredMoistureLevel || head2Moisture < desiredMoistureLevel ) {
      digitalWrite(motorPin, HIGH);
      motorStatusData.motorStatus = true;
      Serial.println("Motor ON (Moisture below threshold)");
    } else {
      digitalWrite(motorPin, LOW);
      motorStatusData.motorStatus = false;
      Serial.println("Motor OFF (Moisture at/above threshold)");
    }

    // Send the motor status data to the specified ESP
   
  } else {
    Serial.println("No change in moisture levels. Motor state unchanged.");
  }
    updateAverageMoistureValues(); 
    esp_now_send(targetMacAddress, (uint8_t *)&motorStatusData, sizeof(motorStatusData));
}

// Function to read battery voltage and calculate percentage
void readBatteryStatus() {
  // Read the analog value from the A0 pin
  int adc = analogRead(A0);

  // Calculate the battery voltage from the ADC reading
  batteryVoltage = (adc - 104) / 15.4;

  // Calculate battery percentage (assuming 10.5V as 0% and 12.6V as 100%)
  batteryPercentage = ((batteryVoltage - 9) / (12.6 - 10.5)) * 100;
  batteryPercentage = constrain(batteryPercentage, 0, 100);

  // Print voltage and percentage to Serial Monitor
  Serial.print("ADC : : ");
  Serial.print(adc);
  Serial.print(" - Battery Voltage: ");
  Serial.print(batteryVoltage, 2);
  Serial.print("V, Battery Level: ");
  Serial.print(batteryPercentage);
  head3Moisture=batteryPercentage;
  Serial.println("%");
}

void setup() {
  Serial.begin(115200);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, HIGH);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed on Motor ESP");
    return;
  }

  // Set the role and register callback functions
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);

  // Add peer with the target MAC address
  esp_now_add_peer(targetMacAddress, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);

  Serial.println("ESP-NOW initialized on Motor ESP");
}

void loop() {
  // Check battery status every second
  readBatteryStatus();
  delay(10000);
  // Motor ESP is always listening for incoming data
}