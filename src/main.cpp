#include <Arduino.h>
#include <AALeC-V3.h>
#include <Wire.h>
#include <WiiChuck.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

Accessory nunchuck;

// WLAN / MQTT
const char* ssid = "iPhone";
const char* password = "moneyboy";
const char* mqtt_server = "172.20.10.3";

WiFiClient espClient;
PubSubClient client(espClient);

// Topic, auf das das Auto hört
const char* carTopic = "aalec/car/cmd";

// damit nicht dauernd derselbe Befehl gespammt wird
String lastCommand = "";

// Joystick-Grenzwerte
const int CENTER_MIN = 90;
const int CENTER_MAX = 170;

const int FORWARD_THRESHOLD = 180;
const int BACKWARD_THRESHOLD = 70;
const int LEFT_THRESHOLD = 70;
const int RIGHT_THRESHOLD = 180;

void publishCommand(String cmd) {
  if (cmd != lastCommand) {
    client.publish(carTopic, cmd.c_str());

    Serial.print("Gesendet: ");
    Serial.println(cmd);

    aalec.print_line(2, "CMD: " + cmd);

    lastCommand = cmd;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT: verbinde... ");

    if (client.connect("AALeC_Controller")) {
      Serial.println("OK");
    } else {
      Serial.print("FEHLER rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  aalec.init();

  aalec.print_line(1, "AALeC Controller");

  Wire.begin();
  Wire.setClock(100000);
  nunchuck.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("WLAN: verbinde");
  aalec.print_line(2, "WLAN...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WLAN verbunden");
  Serial.print("AALeC-IP: ");
  Serial.println(WiFi.localIP());

  aalec.print_line(2, "WLAN OK");

  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  nunchuck.readData();

  int x = nunchuck.getJoyX();
  int y = nunchuck.getJoyY();

  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" | Y: ");
  Serial.println(y);

  String command = "stop";

  // Fehlerhafte Reads ignorieren
  if (x == 0 && y == 0) {
    Serial.println("Ungueltiger Nunchuck-Read ignoriert");
    delay(100);
    return;
  }

  if (y > FORWARD_THRESHOLD) {
    command = "forward";
  } else if (y < BACKWARD_THRESHOLD) {
    command = "backward";
  } else if (x < LEFT_THRESHOLD) {
    command = "left";
  } else if (x > RIGHT_THRESHOLD) {
    command = "right";
  } else {
    command = "stop";
  }

  publishCommand(command);

  delay(100);
}