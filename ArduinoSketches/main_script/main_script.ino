#include "/home/hardal/Desktop/secrets/secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
//sudo chmod 666 /dev/ttyUSB0
// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
int counter=1;
int LED_BUILTIN = 2;
WiFiClientSecure net = WiFiClientSecure();
//PubSubClient MQTTClient(256);
MQTTClient client = MQTTClient(256);
char payloadBuffer[512];
const int trigPin = 13;
const int echoPin = 12;
long duration;
int distance;
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
 net.setCACert(AWS_CERT_CA);
 net.setCertificate(AWS_CERT_CRT);
 net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  //Serial.print("result of the AWS IOT ENDPOINT CONNECTION FROM PORT 8883: ");
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(client.connect(THINGNAME));
    Serial.print("heee");
    delay(1000);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
  delay(500);
digitalWrite(trigPin, LOW);
delayMicroseconds(2);

// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

// Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);

// Calculating the distance
distance= duration*0.034/2;
  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["text"] = "the distance calculated is: ";
  doc["sensor_a0"] = distance;
  
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(String &topic, String &payload) {
  Serial.println("Counter mod2 is: ");
  Serial.println((counter%2));  
  Serial.println("incoming: " + topic + " - " + payload);
  if((counter%2)==0){
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);
  }
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  counter++;
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
pinMode(echoPin, INPUT); 
  pinMode (LED_BUILTIN, OUTPUT);
  connectAWS();
}

void loop() {
  publishMessage();
  client.loop();
  delay(1000);
}
