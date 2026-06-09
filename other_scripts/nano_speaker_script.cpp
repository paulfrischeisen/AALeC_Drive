#include "mp3tf16p.h"

// Pin 10 = RX (geht zu TX am Player), Pin 11 = TX (geht über 1k Widerstand zu RX am Player)
MP3Player mp3(10, 11); 

const int SOUND_TRIGGER_PIN = 2;

// Track-loop
int currentTrack = 1;
const int maxTracks = 3; // maybe mehr hinzufügen später

bool lastTriggerState = LOW;
unsigned long lastTriggerTime = 0;
const unsigned long debounceMs = 300;

void playNextSound() {
  Serial.print(F("Spiele Track: "));
  Serial.println(currentTrack);

  mp3.player.volume(20);
  delay(50);

  mp3.player.play(currentTrack);

  currentTrack++;
  if (currentTrack > maxTracks) {
    currentTrack = 1;
  }

  Serial.print(F("Next Track wäre: "));
  Serial.println(currentTrack);
  Serial.println(F("Play-Befehl gesendet"));
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println();
  Serial.println(F("=============================="));
  Serial.println(F("NANO SOUND TRIGGER CONTROLLER"));
  Serial.println(F("=============================="));

  pinMode(SOUND_TRIGGER_PIN, INPUT);

  // DFPlayer starten
  Serial.println(F("Initialisiere DFPlayer..."));
  mp3.initialize();
  mp3.player.volume(20);
  Serial.println(F("DFPlayer bereit"));

  // kleiner check dass der Lautsprecher noch geht. Einmal sound spielen bei start.
  // playNextSound();

  Serial.println(F("Warte auf Trigger vom D1 mini..."));

  delay(1000);
  lastTriggerState = digitalRead(SOUND_TRIGGER_PIN);

}

void loop() {

  bool triggerState = digitalRead(SOUND_TRIGGER_PIN);

  // Rising Edge erkennen: LOW -> HIGH
  if (triggerState == HIGH && lastTriggerState == LOW) {
    unsigned long now = millis();

    if (now - lastTriggerTime > debounceMs) {
      Serial.println(F("Trigger vom D1 mini empfangen"));
      playNextSound();
      lastTriggerTime = now;
    }
  }

  lastTriggerState = triggerState;
}