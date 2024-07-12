#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include "Wire.h"
#include "String.h"

char ssid[] = "eduroam";         // Replace with WiFi SSID
// char pass[] = "PastaLOL";     // Replace with WiFi Password

// char ssid[] = "DESKTOP-VFKLOHS 0158";         // Replace with WiFi SSID
// char pass[] = "$3N6k456";     // Replace with WiFi Password

#define EAP_ANONYMOUS_IDENTITY "530995@student.saxion.nl" //anonymous@example.com, or you can use also nickname@example.com
#define EAP_IDENTITY "530995@student.saxion.nl" //nickname@example.com, at some organizations should work nickname only without realm, but it is not recommended
#define EAP_PASSWORD "811Watou!" //password for eduroam account
#define EAP_USERNAME "530995" // the Username is the same as the Identity in most eduroam networks.

const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data.
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature, temp; // variables for temperature data

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.

  sprintf(tmp_str, "%6d", i);
  return tmp_str;

}

WiFiClient espClient;
MqttClient mqttClient(espClient);

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org"; // Enter your WiFi or Ethernet IP
const char *topic = "topic/temperature";

void setup() {
  // put your setup code here, to run once:

  // Initialize serial port with standard speed of 115200 bps
  Serial.begin(115200);

  // Connect to WiFi network
  Serial.print("[WiFi]: Connecting to WPA SSID:");
  // connecting to a WiFi network
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  // WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

   Serial.println("[WiFi]: Connection established");
   Serial.println();

  // Connect to MQTT broker
  while (!mqttClient.connect(mqtt_broker, 1883)) {
    Serial.print("[MQTT]: MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    // The connection to the MQTT broker failed, Arduino is halted.
  }

  Serial.println("[MQTT]: Connection established");
  Serial.println();

  // Setup for ITG/MPU

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C. (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050);
  Wire.endTransmission(true);

}

void loop() {
  // put your main code here, to run repeatedly:

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x41); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept alive.
  Wire.requestFrom(MPU_ADDR, 2, true); // request a total of 7*2=14;

  // "Wire.read() << 8 | Wire.read();" means two registers are read and stored in the same variable.
  temperature = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)

  Serial.print("[MQTT]: Sending message to topic: ");

  Serial.println(topic);

  temp = temperature/340.00+36.53;

  Serial.println(convert_int16_to_str(temp));

  Serial.println();

  // Publish message

  mqttClient.beginMessage(topic);

  mqttClient.print(convert_int16_to_str(temp));

  mqttClient.endMessage();

  Serial.println();

}
