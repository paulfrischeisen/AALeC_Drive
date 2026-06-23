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
const char* mqtt_server = "172.20.10.13";

WiFiClient espClient;
PubSubClient client(espClient);

// Topic, auf das das Auto hört
const char* carTopic = "aalec/car/cmd";

// damit nicht dauernd derselbe Befehl gespammt wird
String lastCommand = "";

// den c knopf gedrückt? für Sound
bool lastCPressed = false;
// den z Knopf gedrückt? für Switch
bool lastZPressed = false;

bool useJoyStick = true;

// Joystick-Grenzwerte
const int CENTER_MIN = 90;
const int CENTER_MAX = 170;

const int JOYSTICK_FORWARD_THRESHOLD = 180;
const int JOYSTICK_BACKWARD_THRESHOLD = 70;
const int JOYSTICK_LEFT_THRESHOLD = 70;
const int JOYSTICK_RIGHT_THRESHOLD = 180;

const int TILT_FORWARD_THRESHOLD = -30;
const int TILT_BACKWARD_THRESHOLD = 30;
const int TILT_LEFT_THRESHOLD = -35;
const int TILT_RIGHT_THRESHOLD = 35;

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

void print_xy(int x, int y) {
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" | Y: ");
  Serial.println(y);
}

void getJoystickInput(int& x, int& y) {
  x = nunchuck.getJoyX();
  y = nunchuck.getJoyY();
}
void getTiltInput(int& x, int& y){
  x = nunchuck.getRollAngle();
  y = nunchuck.getPitchAngle();
}
String handleJoystickInput(int x, int y){
  String command = "stop";
  if (y > JOYSTICK_FORWARD_THRESHOLD) {
    command = "forward";
  } else if (y < JOYSTICK_BACKWARD_THRESHOLD) {
    command = "backward";
  } else if (x < JOYSTICK_LEFT_THRESHOLD) {
    command = "left";
  } else if (x > JOYSTICK_RIGHT_THRESHOLD) {
    command = "right";
  } else {
    command = "stop";
  }
  return command;
}
String handleTiltInput(int x, int y){
  String command = "stop";
  if (y < TILT_FORWARD_THRESHOLD) {
    command = "forward";
  } else if (y > TILT_BACKWARD_THRESHOLD) {
    command = "backward";
  } else if (x < TILT_LEFT_THRESHOLD) {
    command = "left";
  } else if (x > TILT_RIGHT_THRESHOLD) {
    command = "right";
  } else {
    command = "stop";
  }
  return command;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  nunchuck.readData();

  bool cPressed = nunchuck.getButtonC();
  bool zPressed = nunchuck.getButtonZ();

  if (cPressed && !lastCPressed) {
    client.publish(carTopic, "sound_next");
    Serial.println("Gesendet: sound_next");
    aalec.print_line(3, "Sound!");
  }

  if (zPressed && !lastZPressed) {
    useJoyStick = !useJoyStick;
  }

  lastCPressed = cPressed;
  lastZPressed = zPressed;

  String command = "stop";

  int x = 0;
  int y = 0;
  if (useJoyStick) {
    getJoystickInput(x, y);
    print_xy(x, y);
    command = handleJoystickInput(x, y);
  }
  else {
    getTiltInput(x, y);
    print_xy(x, y);
    command = handleTiltInput(x, y);
  }

  // Fehlerhafte Reads ignorieren
  if (useJoyStick && x == 0 && y == 0) {
    Serial.println("Ungueltiger Nunchuck-Read ignoriert");
    delay(100);
    return;
  }

  publishCommand(command);

  delay(100);
}