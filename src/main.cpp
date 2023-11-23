#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

char ssid[] = "wifi_name";
const char *password = "wifi_password";
const char *mqttServer = "localhost";
const int mqttPort = 1883;
const char *mqttUsername = "token";
const char *mqttPassword = "";

const char *otaTopic = "v1/devices/me/attributes";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

bool isFirmwareUpgradeTriggered = false;
void connectToWiFi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
}

void connectToMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);
  while (!mqttClient.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP32Client", mqttUsername, mqttPassword))
    {
      Serial.println("Connected to MQTT!");
      mqttClient.subscribe(otaTopic);
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void handleOTAUpdate(const uint8_t *firmwareData, size_t firmwareSize)
{
  ArduinoOTA.setHostname("ESP32Device");
  ArduinoOTA.onStart([]() { 
    Serial.println("OTA Update started..."); });
  ArduinoOTA.onEnd([]() { 
    Serial.println("OTA Update completed!");
    isFirmwareUpgradeTriggered = false;});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { 
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)  {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
    Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println((String)*topic + " " + (String)*otaTopic);
  Serial.println("strcmp = " + (String)strcmp(topic, otaTopic));
  isFirmwareUpgradeTriggered = true;
  if (strcmp(topic, otaTopic) == 0)
  {
    if (!isFirmwareUpgradeTriggered)
    {
      isFirmwareUpgradeTriggered = true;
      handleOTAUpdate(payload, length);
      Serial.println("Firmware upgrade triggered!");
    }
  }
}
void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  connectToMQTT();
  mqttClient.setCallback(callback);
  ArduinoOTA.setPort(3232);
  ArduinoOTA.begin();
}

void loop()
{
  if (!mqttClient.connected())
  {
    connectToMQTT();
  }
  mqttClient.loop();
  if (isFirmwareUpgradeTriggered) ArduinoOTA.handle();
  isFirmwareUpgradeTriggered = false;
  Serial.println("atualização 1.9");
  delay(1000);
}