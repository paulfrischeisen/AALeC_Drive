# Grundvoraussetzungen für das Projekt-Setup

Für die vollständige Inbetriebnahme des Projekts werden folgende Software-Komponenten benötigt:

- **Mosquitto MQTT Broker** (z. B. Download über https://mosquitto.org)
- **Arduino IDE** (inkl. Installation der benötigten Bibliotheken für ESP8266 und Arduino Nano)
- **Visual Studio Code** inkl. PlatformIO Extension (für den AALeC-Microcontroller)

---

# Wichtige Anpassungen in den Skripten

Vor dem Upload der Programme (AALeC_Script & Car_Script) auf die Mikrocontroller müssen folgende Parameter angepasst werden:

### Netzwerk & MQTT (AALeC_Script & Car_Script)
- WLAN-Netzwerkname (SSID) und Passwort im jeweiligen Skript eintragen  
- IPv4-Adresse des MQTT-Brokers im Code hinterlegen  
  - Diese kann über PowerShell mit `ipconfig` ermittelt werden  
- Hinweis: Die IPv4-Adresse kann sich bei vielen Netzwerken regelmäßig (täglich) ändern, weshalb die Skripte ggf. vor jeder Nutzung neu angepasst und auf die Geräte geladen werden müssen
```cpp
const char* ssid = "WLAN_Netzwerk";
const char* password = "WLAN_Passwort";
const char* mqtt_server = "192.168.0.103";
```

### Soundausgabe & Eigene Sounds (Speaker_Script)
Wenn die standardmäßigen Sounds geändert oder erweitert werden sollen, müssen folgende Schritte durchgeführt werden:
1. **SD-Karte bespielen:** Die neuen Sounddateien auf die Micro-SD-Karte des DFPlayer Mini laden.
2. **Maximalanzahl im Code anpassen:** Da der Arduino Nano die Track-Reihenfolge selbstständig verwaltet, muss im `Speaker_Script` (für den Nano) die Variable für die maximale Anzahl der Tracks angepasst werden:

   ```cpp
   const int maxTracks = 3;
---

# Deployment auf die Mikrocontroller

Die einzelnen Programme werden wie folgt auf die Geräte übertragen:

- **Car_Script** → Upload auf den D1 mini des Fahrzeugs  
  - Verbindung über Micro-USB-Kabel  
  - Upload erfolgt über die Arduino IDE  
  - Wichtig: Als Board muss **LOLIN(WEMOS) D1 mini** ausgewählt werden  

- **Speaker_Script** → Upload auf den Arduino Nano (Audioausgabe des Fahrzeugs)  
  - Verbindung über USB-C
  - Upload über die Arduino IDE
  - Header-File `mp3tf16p.h` wird benötigt  

- **AALeC_Script** → Upload auf den AALeC-Microcontroller  
  - Verbindung über USB-C  
  - Upload erfolgt über PlatformIO in Visual Studio Code

---

# Starten des MQTT-Brokers (Mosquitto)

Zu Beginn muss in das Installatationsverzeichnis von Mosquitto navigiert werden. Standardmäßig über den Befehl:

```powershell
cd "C:\Program Files\mosquitto"
```

Für die Kommunikation zwischen den D1-Mini-Clients und dem Server wird ein lokaler MQTT-Broker mit **Mosquitto** gestartet. Dies erfolgt über den folgenden Befehl:

```powershell
.\mosquitto.exe -c "C:\...\Example_Project-main\other_scripts\mosquitto_wlan.conf.txt" -v
```

Dabei lädt die Option `-c` die Konfigurationsdatei `mosquitto_wlan.conf.txt`, während `-v` den Broker im **Verbose-Modus** startet und sämtliche Verbindungen sowie MQTT-Nachrichten in der Konsole ausgibt.

Die verwendete Konfiguration lautet:

```conf
listener 1883 0.0.0.0
allow_anonymous true
```

Die Einstellung `listener 1883 0.0.0.0` bewirkt, dass der Broker auf dem Standard-MQTT-Port **1883** auf allen Netzwerkinterfaces lauscht und somit von anderen Geräten im selben Netzwerk erreichbar ist. Mit `allow_anonymous true` wird der Zugriff ohne Benutzername und Passwort erlaubt. Dadurch können sich die D1-Mini-Clients direkt mit dem Broker verbinden. Diese Konfiguration eignet sich für Test- und Entwicklungsumgebungen im lokalen Netzwerk, sollte jedoch aus Sicherheitsgründen nicht in produktiven Umgebungen verwendet werden.

## Hinweis zum MQTT-Broker

Der MQTT-Broker (Mosquitto) wird auf einem **WLAN-fähigen Host-Gerät (z. B. Laptop)** im lokalen Netzwerk ausgeführt.

Alle Mikrocontroller (D1 mini, Arduino Nano, AALeC) verbinden sich anschließend über dieses Netzwerk mit dem Broker. Voraussetzung ist, dass sich alle Geräte im **gleichen WLAN-Netzwerk** befinden.

---

# Physikalisches Setup

Nach erfolgreichem Flashen der Mikrocontroller müssen folgende physische Komponenten verbunden und aktiviert werden:

## Hardware-Verbindungen

- Der **Wii Nunchuck** muss an den AALeC-Microcontroller angeschlossen werden  
- Das Auto wird über den **Ein-/Ausschalter** eingeschaltet (Stromversorgung der Motoren über das Batteriefach)  
- Der D1 mini des Autos wird über eine **Powerbank via Micro-USB-Kabel** mit Strom versorgt (Versorgung des Lautsprechers)

---

# Steuerung des Fahrzeugs

Die Steuerung erfolgt über den Wii Nunchuck:

- Der **Joystick** steuert die Fahrbewegung des Fahrzeugs  
- Mit der **C-Taste** wird ein Hup-Sound ausgelöst  
- Mit der **Z-Taste** wird zwischen zwei Steuerungsmodi gewechselt:  
  - Joystick-Steuerung  
  - Neigungssteuerung (Tilt-Control)  

Ein erneutes Drücken der Z-Taste wechselt wieder zurück in den vorherigen Modus.