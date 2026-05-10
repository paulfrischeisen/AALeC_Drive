#include <Arduino.h>
#include <AALeC-V3.h>
#include <Wire.h>
#include <WiiChuck.h>

Accessory nunchuck;

void setup() {
  Serial.begin(115200);
  delay(1000);

  aalec.init();

  Serial.println("AALeC Nunchuck Test");
  aalec.print_line(1, "Nunchuck Test");

  Wire.begin();
  Wire.setClock(100000);
  nunchuck.begin();
}

void loop() {
  nunchuck.readData();

  for (int i = 0; i < 12; i++) {
    Serial.print("v[");
    Serial.print(i);
    Serial.print("]=");
    Serial.print(nunchuck.values[i]);
    Serial.print("  ");
  }

  Serial.println();
  Serial.print("X-Input: ");
  Serial.print(nunchuck.getJoyX());
  Serial.print("\t Y-Input: ");
  Serial.print(nunchuck.getJoyY());
  Serial.println();
  delay(1000);
}