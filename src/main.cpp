#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "Wire.h"
#include "DHT20.h"
#include <ArduinoJson.h>
#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>



const int soilPin = 33; // Analog input pin for AO
#define LED_PIN 2
#define TEMPERATUR_LIGHT 12
DHT20 dht20;
const float warning_temp = 27;

#define WIFI_SSID "iPhone (9)" // NOTE: Please delete this value before submitting assignment
#define WIFI_PASSWORD "HomeworkUseOnly" // NOTE: Please delete this value before submitting assignment

// Azure IoT Hub configuration
#define SAS_TOKEN "SharedAccessSignature sr=cs147Lab4IotHub.azure-devices.net%2Fdevices%2F147esp32&sig=jKIjoAvGack09RIXzCnLYl05DKEBZJinozhsZUqR8wo%3D&se=1757103512"
// Root CA certificate for Azure IoT Hub
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n" \
"MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n" \
"UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n" \
"eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n" \
"GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n" \
"mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n" \
"SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n" \
"dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n" \
"AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
"VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n" \
"buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
"AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n" \
"b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n" \
"Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n" \
"oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n" \
"dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n" \
"AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n" \
"klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n" \
"kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n" \
"eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n" \
"penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n" \
"QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n" \
"q/xKzj3O9hFh/g==\n" \
"-----END CERTIFICATE-----\n";


String iothubName = "cs147Lab4IoTHub"; //hub name
String deviceName = "147esp32"; //device name
String url = "https://" + iothubName + ".azure-devices.net/devices/" + deviceName + "/messages/events?api-version=2021-04-12";

// Telemetry interval
#define TELEMETRY_INTERVAL 3000 // Send data every 3 seconds


uint8_t count = 0;
uint32_t lastTelemetryTime = 0;

void setup() {
  Serial.begin(9600);
  delay(1000); // Give time for Serial Monitor to connect
  Wire.begin();          // SDA=21, SCL=22 (ESP32)
  WiFi.mode(WIFI_STA);

  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);


  if (!dht20.begin()) {        // true = OK
    Serial.println("DHT20 init failed!");
    while (1) 
    delay(1000);
  }
  else {
    Serial.println("DHT20 init worked!");
  }
  Serial.println("Soil Moisture Sensor Test");


  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMPERATUR_LIGHT, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(TEMPERATUR_LIGHT, LOW);


  WiFi.disconnect(true);
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Wait until connected to Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }

  
}

void loop() {
  int sensorValue = analogRead(soilPin); // Read raw analog value (0-4095 on ESP32)

  dht20.read();
  float T = dht20.getTemperature();
  float H = dht20.getHumidity();

  
  Serial.printf("T=%.2f°C  H=%.2f%%\n", T, H);
  if(T >= warning_temp){
    Serial.println("Temperauture is too high!");
    digitalWrite(TEMPERATUR_LIGHT, HIGH);
  } else{
    digitalWrite(TEMPERATUR_LIGHT, LOW);
  }

  Serial.print("soilMoisture: ");
  Serial.println(sensorValue);

  if (sensorValue > 3000) { 
    Serial.println("Soil is dry!");
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("Soil is moist");
    digitalWrite(LED_PIN, LOW);
  }

  Serial.println("------");

// Create JSON payload
  ArduinoJson::JsonDocument doc;
  doc["temperature"] = T;
  doc["humidity"] = H;
  doc["soilMoisture"] = sensorValue;
  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));

 // Send telemetry via HTTPS
  WiFiClientSecure client;
  client.setCACert(root_ca); // Set root CA certificate
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", SAS_TOKEN);
  int httpCode = http.POST(buffer);

  if (httpCode == 204) { // IoT Hub returns 204 No Content for successful telemetry
      Serial.println("Telemetry sent: " + String(buffer));
  } else {
      Serial.println("Failed to send telemetry. HTTP code: " + String(httpCode));
  }
  http.end();

  // Example threshold - adjust based on calibration
  
  delay(2000); // Read every 2 seconds
}