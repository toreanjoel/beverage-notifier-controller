/**
 * Author: Torean Joel - https://github.com/toreanjoel
 * 
 * Description:
 * ---
 * Power up then advertise to connect to a device, get temps using IR sensor.
 * We then go to a deep sleep for a set duration.
 * All this happens once the device is powered on and has a toggle to do that.
 * 
 * Note: This has hard-coded UUIDs and needs to change for each physical device.
*/

// Include necessary libraries
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_sleep.h>

// Define UUIDs for service and characteristic
// This is 128 bit identifier, you can generate one here: https://www.uuidgenerator.net
// This is unique per server and characteristic and you need to setup the serivices with these UUIDs
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define INDICATOR_LED 2 // The indicator led that will be used to show state of advertising etc
#define TEMP_SENSOR 15 // Pin connected to the LM35 sensor

#define SLEEP_DURATION 15 // Sleep duration in seconds
#define NUM_READINGS 10 // Number of temperature readings to average
#define ADC_RESOLUTION 4095 // The maximum digital value that the ADC can output (12 bits)
#define V_REF 3.3  // Reference voltage for the ADC on ESP32
#define LM35_SCALE 0.01 // LM35 outputs 0.01 volts for each degree Celsius
#define ADVERTISING_TIME 60 // Advertising iterations
#define PROCESS_NOTIFICAION_TIME 1000 // Process notification time in milliseconds

// Declare pointers for characteristic and advertising objects
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;
bool deviceConnected = false;  // Variable to keep track of connections

// Declare a class for server callbacks
class MyServerCallbacks: public BLEServerCallbacks {
  // This function will be called when a device connects
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;  // Update the connection status
    Serial.println("Connected to a device");
  }

  // This function will be called when a device disconnect
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false; // Update the connection status
    Serial.println("Disconnected from a device");
  }
};

void setup() {
  // Set the pin modes for the relevant modes
  pinMode(TEMP_SENSOR, INPUT);  // Set the GPIO pin as input
  pinMode(INDICATOR_LED, OUTPUT); // LED indicator pin state

  // Serial rate - helps with reading serial data
  Serial.begin(9600);

  // Set up device name
  String device_name = "crud.sh::beverage_notifier";
  
  // // Device name as a identifier
  BLEDevice::init(device_name.c_str()); // Initialize the BLE device - needs to be a char list character C string array

  // // Create a BLE server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks()); // Set the server callbacks

  // // Create a BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // // Create a BLE characteristic
  pCharacteristic =
    pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      // This allows us to have the server send data when it changes to a characteristic
      // This can be thought of the way the server <-> client are allowed to interact (rules)
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_READ
    );
  
  // // The descriptor to enable and configure if a device needs to get notified when updates happen
  pCharacteristic->addDescriptor(new BLE2902());
  // // Start advertising the service
  pAdvertising = pServer->getAdvertising();

  // // changes the limit of characters at which we can use to broadcast the device name
  pAdvertising -> setMinPreferred(0x0);

  // // Start the service - async?
  pService->start();

  // Start the advertising for a certain duraion
  // advertise that allows advertising untill a user is connected or 30s pass (what ever comes first)
  for (int i = 0; i < ADVERTISING_TIME; i++) {
    if(deviceConnected) {
      // break out the advertising once we know there is a client connected
      break;
    } else { // do we need to do this??
      // advertise
      digitalWrite(INDICATOR_LED, HIGH);
      pAdvertising->start();
      // keep trying - we wait before trying to advertise again
      delay(1000); // this results in 60 iterations with 1s pause being a 60s advertising time at the most
    }
  }

  // // Set the led to go on when we are advertising - this will turn off as the device sleeps
  digitalWrite(INDICATOR_LED, LOW);

  // If connected, read temperature and send data
  // ---
  if (deviceConnected) {
    float total = 0;  // Variable to hold the total temperature

    // Prepare to go to sleep
    Serial.println("Prepare getting averages...");
    // Read the temperature NUM_READINGS times
    for (int i = 0; i < NUM_READINGS; i++) {
      // The analogRead function returns a digital value between 0 and ADC_RESOLUTION
      int reading = analogRead(TEMP_SENSOR);
      // Convert the reading back into a voltage
      float voltage = reading * (V_REF / ADC_RESOLUTION);
      // // Convert the voltage into a temperature
      float temperature = voltage / LM35_SCALE;
      total += temperature; // Add the temperature to the total
      delay(10);
    }

    float average = total / NUM_READINGS; // Compute the average temperature
    String value = String(average);
    
    // Send the average temperature
    pCharacteristic->setValue(value.c_str()); // converting into a c-style string (null terminated character array)
    pCharacteristic->notify(); // this is needed if there is a rule set to send the data
    Serial.print(value.c_str());
    // We pause before we sleep the system as we need to wait for notify to be enabled and sent
    // Bluetooth stack will go into congestion - we only need 3ms but we wait longer for now 
    delay(PROCESS_NOTIFICAION_TIME);
  }

  // Sleep for SLEEP_DURATION seconds
  // ---
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000);
  esp_deep_sleep_start();
}

void loop() {
  // Not required for V1 of application - system starts up and sleeps for duration then starts again
}