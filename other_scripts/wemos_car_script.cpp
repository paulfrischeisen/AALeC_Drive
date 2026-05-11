#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "iPhone";
const char* password = "moneyboy";
const char* mqtt_server = "172.20.10.3";

#define IN1 D5
#define IN2 D6
#define IN3 D7
#define IN4 D0

WiFiClient espClient;
PubSubClient client(espClient);

void stopMotors() {
  Serial.println("MOTOR: stop");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {
  Serial.println("MOTOR: forward");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Rückwärts linkes Rad probleme. Input PINs prüfen am wemos!! Verhält sich komisch
// Muss an IN2 bzw. IN4 eigentlich liegen (forwärts geht alles)
// Insgesamt ist die Verbindung manchmal am hängen (vielleicht auch Kabel die teilweise lose werden oder WLAN Hotspot)
void backward() {
  Serial.println("MOTOR: backward");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  Serial.println("MOTOR: left");
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  Serial.println("MOTOR: right");
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// mqtt callback, wird automatisch aufgerufen, sobald eine mqtt nachricht ankommt (zb forward, right, ..)
// mit .\mosquitto_pub.exe -h 172.20.10.3 -t aalec/car/cmd -m "forward" kann man dann zb forward an den wemos am auto senden
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  Serial.print("MQTT Topic: ");
  Serial.println(topic);

  Serial.print("MQTT Payload: ");
  for (unsigned int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
    Serial.print(c);
  }
  Serial.println();

  msg.trim();

  if (msg == "forward") forward();
  else if (msg == "backward") backward();
  else if (msg == "left") left();
  else if (msg == "right") right();
  else if (msg == "stop") stopMotors();
  else {
    Serial.print("Unbekannter Befehl: ");
    Serial.println(msg);
    stopMotors();
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT: verbinde zu ");
    Serial.print(mqtt_server);
    Serial.print(" ... ");

    if (client.connect("AutoWemos")) {
      Serial.println("OK");
      // Wenn connection geklappt hat, hört der wemos auf das topic aalec/car/cmd
      client.subscribe("aalec/car/cmd");
      Serial.println("MQTT: subscribed auf aalec/car/cmd");
      stopMotors();
    } else {
      Serial.print("FEHLER rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

// Im Setup wird nacheinander das gemacht: Serial Monitor starten -> Motorpins als output setzen -> motoren stoppen (falls sie anfangs an sind)
// wlan verbindung aufbauen (hotspot von Paul) -> ip-adresse ausgeben -> mqtt broker und callback registrieren.
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("==============================");
  Serial.println("AUTO-WEMOS DEBUG START");
  Serial.println("==============================");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("WLAN: verbinde mit ");
  Serial.println(ssid);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;

    if (counter > 60) {
      Serial.println();
      Serial.println("WLAN: Timeout, starte neu...");
      ESP.restart();
    }
  }

  Serial.println();
  Serial.println("WLAN: verbunden");
  Serial.print("Wemos-IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}