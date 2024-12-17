#define BLYNK_TEMPLATE_ID "TMPL3JQx88TIF"
#define BLYNK_TEMPLATE_NAME "smart irrigation"
#define BLYNK_AUTH_TOKEN "hZkrqBY4GPZr79f-7t1QW4468Thzui5X" 

#include <ESP8266WiFi.h>     // Use <WiFi.h> for ESP32
#include <espnow.h>
#include <BlynkSimpleEsp8266.h>  // Use <BlynkSimpleEsp32.h> for ESP32

/// Blynk credentials
char auth[] = "hZkrqBY4GPZr79f-7t1QW4468Thzui5X";
const char* ssid = "2601";         // Replace with your WiFi SSID
const char* password = "02072005";

int head1;
int head2;
int head3;
int motorS;
int desiredMoistureLevel = 65;


typedef struct {
  int avgMoistureHead1;
  int avgMoistureHead2;
  int avgMoistureHead3;
  bool motorStatus;
} MotorStatusData;

MotorStatusData receivedData;



// Flag to check if data is received
bool dataReceived = false;

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  // Check if the received data is the expected size
  if (len == sizeof(MotorStatusData)) {
    // Copy the received data into our struct
    memcpy(&receivedData, incomingData, sizeof(receivedData));

    // Print the received data to the Serial Monitor
    Serial.println("Received data from Motor ESP:");
    Serial.print("Average Moisture Head 1: ");
    Serial.println(receivedData.avgMoistureHead1);
    head1=receivedData.avgMoistureHead1;
    Serial.print("Average Moisture Head 2: ");
    Serial.println(receivedData.avgMoistureHead2);
    head2=receivedData.avgMoistureHead2;
    Serial.print("Average Moisture Head 3: ");
    Serial.println(receivedData.avgMoistureHead3);
    head3=receivedData.avgMoistureHead3;
    Serial.print("Motor Status: ");
    Serial.println(receivedData.motorStatus ? "ON" : "OFF");
    dataReceived = true;
if(head1 < desiredMoistureLevel || head2 < desiredMoistureLevel){
  motorS=1;
}
else{
  motorS=0;
}
  


  } else {
    Serial.println("Error: Data received has incorrect size. Ignoring data.");
  }
// Callback function to handle received ESP-NOW data
}

// Function to initialize ESP-NOW
void initializeEspNow() {
    WiFi.mode(WIFI_AP_STA);       // Set WiFi to Station mode for ESP-NOW
    WiFi.disconnect();         // Disconnect from any existing WiFi connection

    if (esp_now_init() != 0) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
    esp_now_register_recv_cb(onDataRecv);  // Register receive callback
    Serial.println("ESP-NOW initialized");
}

// Function to send data to Blynk over WiFi
void sendDataToBlynk() {
    // Disable ESP-NOW before connecting to WiFi
    esp_now_deinit();

    // Connect to WiFi
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");   

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println(" Connected to WiFi");

    // Initialize Blynk connection
    Blynk.config(auth);
    Blynk.connect();

   
       if (Blynk.connected()) {
        Blynk.virtualWrite(V0,head1); 
          Serial.print("Data sent to Blynk:1 = ");
        Serial.println(head1); // Send data to Blynk virtual pin V1
         Blynk.virtualWrite(V1,head2); 
          Serial.print("Data sent to Blynk: 2 =  ");
        Serial.println(head2);
          Blynk.virtualWrite(V2,motorS); 
          Serial.print("Data sent to Blynk: motor = ");
        Serial.println(motorS);
        Blynk.virtualWrite(V3,head3); 
          Serial.print("Data sent to Blynk: battery = ");
        Serial.println(head3);
           // Send data to Blynk virtual pin V1
       }


    // Disconnect WiFi and fully reinitialize ESP-NOW
    Blynk.disconnect();
    WiFi.disconnect(true);    // true parameter forces WiFi to fully disconnect
    delay(100);               // Short delay to ensure WiFi is fully disabled
    initializeEspNow();       // Reinitialize ESP-NOW after WiFi operation
    Serial.println("Returned to ESP-NOW mode");
        ESP.restart();
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Start with ESP-NOW
    initializeEspNow();
}

void loop() {
    if (dataReceived) {
        // Send received data to Blynk and reset the flag
        sendDataToBlynk();
        dataReceived = false;
    }

    // Regularly call Blynk.run() if WiFi is connected (optional)
    if (WiFi.status() == WL_CONNECTED) {
        Blynk.run();
    }
}