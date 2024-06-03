#include <WiFiNINA.h>
#include <PubSubClient.h>

// WiFi and MQTT Broker Credentials
const char* ssid = "RHD32"; 
const char* password = "12345678";
const char* mqttBroker = "broker.emqx.io";
const int mqttPort = 1883;
int waves = 0;

// Your Name
const char* Name = "Ashen";

//IFTTT integration
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/10_waves_detected/with/key/k1V6CeoBhVqGaVbQQeOdp72fTHXsY1uuuDr0rXQ09c4";


// Ultrasonic Sensor Pins
const int trigPin = 2;  
const int echoPin = 3;  

// LED Pin
const int ledPin = 4;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // WiFi Setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // MQTT Setup
  mqttClient.setServer(mqttBroker, mqttPort);
  mqttClient.setCallback(callback); 

  connectToMQTT();
}

void loop() {
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop(); 

  detectAndPublishWaveOrPat();

  if ( waves == 10){
    emailSender();
  }
}

void detectAndPublishWaveOrPat() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  Serial.println(distance);

  if (distance > 10 && distance < 30) { // Threshold for wave detection
    char message[50];
    snprintf(message, 50, "%s sent a wave!", Name);
    mqttClient.publish("SIT210/wave", message); 
  }
  if (distance < 10) { // Threshold for pat detection 
    char message[50];
    snprintf(message, 50, "%s sent a pat!", Name);
    mqttClient.publish("SIT210/wave", message); 
  }

  delay(500); 
}






void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect("your_arduino_client")) { // Client ID
      Serial.println("Connected to MQTT");
      mqttClient.subscribe("SIT210/wave"); 
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  // Convert payload to string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (message.indexOf("pat") >= 0) {
    // Blink LED in a different pattern if message contains "pat"
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(200); // longer ON time
      digitalWrite(ledPin, LOW);
      delay(200); // longer OFF time
    }
  } else {
    //update detctted wave amount
    waves++;
    // Default blink pattern
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(500); // shorter ON time
      digitalWrite(ledPin, LOW);
      delay(500); // shorter OFF time
    }
  }
}

void emailSender(){
    if (wifiClient.connect(HOST_NAME, 80)) {
        // if connected:
      Serial.println("Connected to server");
    }
    else {// if not connected:
      Serial.println("connection failed");
    }
    //trigerring the 10 wave detection available email
    // make a HTTP request:
    // send HTTP header
    wifiClient.println("GET " + PATH_NAME + " HTTP/1.1");
    wifiClient.println("Host: " + String(HOST_NAME));
    wifiClient.println("Connection: close");
    wifiClient.println(); // end HTTP header

    while (wifiClient.connected()) {
      if (wifiClient.available()) {
        // read an incoming byte from the server and print it to serial monitor:
        char c = wifiClient.read();
        Serial.print(c);
      }
    }
    // the server's disconnected, stop the client:
    wifiClient.stop();
    Serial.println();
    Serial.println("disconnected");
    waves = 0;
}