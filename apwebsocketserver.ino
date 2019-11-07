#include <WiFiNINA.h>
#include <WebSocketServer.h>
#include <Arduino_LSM6DS3.h>
#include <RTCZero.h>
#include <Base64.h>
#include <FlashStorage.h>
#include "wifi_secrets.h"

//#define DEBUG 1

#ifdef DEBUG
  #define Sprint(a) (Serial.print(a))
  #define Sprintln(a) (Serial.println(a))
#else
  #define Sprint(a)
  #define Sprintln(a)
#endif

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;
const IPAddress APIP(192, 168, 2, 1);
int APStatus = WL_IDLE_STATUS;
const int APChannel = 13;
const int webPort = 80;
const int socketPort = 8080;
WiFiServer webServer(webPort);
WiFiServer socketServer(socketPort);
WebSocketServer webSocketServer;
WiFiClient socketClient;
RTCZero rtc;
const int ledPin = 12;
const int WiFiSwitchPin = 2;
boolean turnWiFiOn = false;
boolean WiFiSwitchState = HIGH;
unsigned long debounceStart = 0;
const int debounceDelay = 1000;
boolean previousWiFiSwitchState = HIGH;
boolean on = false;
float volume = 0;
int cowbell = 1000;
typedef struct {
  boolean settingsSaved_F;
  boolean on_F;
  float volume_F;
  int cowbell_F;
} Settings;
FlashStorage(flashStore, Settings);
Settings settings;
float zdata[] = {1,1,1,1,1};
int zdatasize = sizeof(zdata) / sizeof(zdata[0]);
float tdata[] = {20,20,20,20,20,20,20,20,20,20};
int tdatasize = sizeof(tdata) / sizeof(tdata[0]);
int ledState = 0;
unsigned long previousCowbellMillis = 0;
unsigned long previousGravityMillis = 0;
unsigned long previousTemperatureMillis = 0;
const char* interface_gz_base64 = "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";

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);
  #endif
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(WiFiSwitchPin, INPUT_PULLUP);
  IMU.begin();
  rtc.begin();
  settings = flashStore.read();
  if (settings.settingsSaved_F) {
    on = settings.on_F;
    volume = settings.volume_F;
    cowbell = settings.cowbell_F;
  }
}

void loop() {

  unsigned long currentCowbellMillis = millis();
  if ((currentCowbellMillis - previousCowbellMillis) > cowbell) {
    previousCowbellMillis = currentCowbellMillis;
    if (on) {
      if (cowbell == 1000) {
        analogWrite(ledPin, (int)round(volume));
      } else {
        if (ledState == 0) {
          ledState = 1;
          analogWrite(ledPin, (int)round(volume));
        } else {
          ledState = 0;
          analogWrite(ledPin, 0);
        }
      }
    } else {
      analogWrite(ledPin, 0);
    }
  }

  WiFiSwitchState = digitalRead(WiFiSwitchPin);
  if (WiFiSwitchState == LOW && previousWiFiSwitchState == HIGH && (millis() - debounceStart) > debounceDelay) {
    turnWiFiOn = !turnWiFiOn;
    debounceStart = millis();
  }
  previousWiFiSwitchState = WiFiSwitchState;

  if ((WiFi.status() == WL_AP_CONNECTED || WiFi.status() == WL_AP_LISTENING) && !turnWiFiOn) {
    if (socketClient.connected()) {
      webSocketServer.disconnectStream();
      socketClient.stop();
    }
    WiFi.end();
    digitalWrite(LED_BUILTIN, LOW);
    Sprintln("\n--Access Point switched off");
    settings.on_F = on;
    settings.volume_F = volume;
    settings.cowbell_F = cowbell;
    settings.settingsSaved_F = true;
    flashStore.write(settings);
    Sprintln("--Settings saved to flash");
    return;
  } else if (WiFi.status() != WL_AP_CONNECTED && WiFi.status() != WL_AP_LISTENING && !turnWiFiOn) {
    return;
  } else if (WiFi.status() != WL_AP_CONNECTED && WiFi.status() != WL_AP_LISTENING && turnWiFiOn) {
    APSetup();    
    return;
  }

  #ifdef DEBUG
  if (APStatus != WiFi.status()) {
    APStatus = WiFi.status();
    if (APStatus == WL_AP_CONNECTED) {
      Sprintln("\n--A device connected to the Access Point");
    } else {
      Sprintln("\n--A device disconnected from the Access Point");
    }  
  }
  #endif

  WiFiClient webClient = webServer.available();
  if (webClient.connected()) {
    Sprint("\n--New client: "); Sprint(webClient.remoteIP()); Sprint(":"); Sprintln(webClient.remotePort());
    String header = "";
    unsigned long requestStartMillis = millis();
    while (webClient.available()) {
      unsigned long requestCurrentMillis = millis();
      char c = webClient.read();
      if (c != '\r') {
        header += c;
      }
      if ((requestCurrentMillis - requestStartMillis) > 5000) {
        Sprintln("--Request is taking too long");
        webClient.println("HTTP/1.1 408 Request Timeout\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n408 Request Timeout\n");
        break;
      } else if (header.length() == 3 && header != "GET") {
        Sprintln("--Wrong method in header");
        webClient.println("HTTP/1.1 405 Method Not Allowed\nAllow: GET\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n405 Method not allowed; GET only\n");
        break;
      } else if (header.indexOf("pgrade") > -1 && header.indexOf("ebsocket") > -1) {
        Sprintln("--Websocket upgrade requested on webserver port");
        webClient.println("HTTP/1.1 406 Not Acceptable\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n406 Not Acceptable; websocket not available on this port\n");
        break;
      } else if (header.indexOf("event-stream") > -1) {
        Sprintln("--SSE content type request in header");
        webClient.println("HTTP/1.1 406 Not Acceptable\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n406 Not Acceptable; SSE not available\n");
        break;
      } else if (header.length() > 512) {
        Sprintln("--Header too long");
        webClient.println("HTTP/1.1 431 Request Header Fields Too Large\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n431 Request header fields too large\n");
        break;
      } else if (!webClient.available() && (header.length() <= 10 || (header.length() > 10 && header.substring(header.length() - 2) != "\n\n"))) {
        Sprintln("--Incomplete header");
        webClient.println("HTTP/1.1 400 Bad Request\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n400 Bad Request; incomplete header\n");
        break;
      } else if (header.substring(header.length() - 2) == "\n\n") {
        Sprint(header.substring(0, header.length() - 1));
        if (header.indexOf("GET / HTTP") > -1) {
          sendBase64Page(interface_gz_base64, webClient, 1024);
          Sprintln("--Interface webpage sent");
        } else if (header.indexOf("GET /author HTTP") > -1) {
          webClient.println("HTTP/1.1 200 OK\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\nOscar den Uijl, oscar@den-uijl.nl\n");
          Sprintln("--Email address sent");
        } else if (header.indexOf("GET /coffee HTTP") > -1) {
          webClient.println("HTTP/1.1 418 I'm a teapot\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n418 I'm a teapot\n");
          Sprintln("--Coffee request denied");
        } else {
          webClient.println("HTTP/1.1 404 Not Found\nConnection: close\nContent-Type: text/plain; charset=utf-8\n\n404 Not Found\n");
          Sprintln("--Page not found");
        }
      } 
    }
    webClient.stop();
    Sprintln("--Client disconnected");
    return;
  }

  if (!socketClient.connected()) {
    socketClient = socketServer.available();
    if (socketClient.connected() && webSocketServer.handshake(socketClient)) {
      Sprint("\n--Websocket connected to: "); Sprint(socketClient.remoteIP()); Sprint(":"); Sprintln(socketClient.remotePort());
      if (on) {
        webSocketServer.sendData("sw:true");
      } else {
        webSocketServer.sendData("sw:false");
      }
      webSocketServer.sendData("volume:" + (String)round((volume*100)/255));
      webSocketServer.sendData("cowbell:" + (String)((1000-cowbell)/10));
      Sprintln("--Settings sent");
    } else {
      socketClient.stop();
    }
  }

  if (socketClient.connected()) {
    String data = webSocketServer.getData();
    if (data.length() > 0) {
      String name = data.substring(0, data.indexOf(":"));
      String value = data.substring(data.indexOf(":") + 1);
      boolean goodSettings = true;
      if (name == "time") {
        rtc.setEpoch(strtoul(value.c_str(), NULL, 10));
      } else if (name == "switch") {
        on = (value == "true");
      } else if (name == "volume") {
        volume = (value.toFloat()*255)/100;
        if (cowbell == 1000) {
          analogWrite(ledPin, (int)round(volume));
        }
      } else if (name == "cowbell") {
        cowbell = 1000 - (value.toInt()*10);
      } else {
        goodSettings = false;
        webSocketServer.sendData("message:Bad data; ignored");
      }
      if (goodSettings) {
        webSocketServer.sendData("message:" + name + " set to " + value);
      }
    }
  }

  float t, tsum = 0;
  if (IMU.temperatureAvailable()) {
    IMU.readTemperature(t);
    for (int i = 0; i < tdatasize - 1; i++) {
      tdata[i] = tdata[i+1];
    }
    tdata[tdatasize-1] = t;
    for (int i = 0; i < tdatasize; i++) {
      tsum += tdata[i];
    }
    if (socketClient.connected()) {
      unsigned long currentTemperatureMillis = millis();
      if ((currentTemperatureMillis - previousTemperatureMillis) > 5000) {
        previousTemperatureMillis = currentTemperatureMillis;
        webSocketServer.sendData("temperature:" + String(tsum/tdatasize, 3));
      }
    }
  }
  
  float x, y, z, zsum = 0;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    for (int i = 0; i < zdatasize - 1; i++) {
      zdata[i] = zdata[i+1];
    }
    zdata[zdatasize-1] = z;
    for (int i = 0; i < zdatasize; i++) {
      zsum += zdata[i];
    }
    if (socketClient.connected()) {
      unsigned long currentGravityMillis = millis();
      if ((currentGravityMillis - previousGravityMillis) > 2000) {
        previousGravityMillis = currentGravityMillis;
        webSocketServer.sendData("gravity:" + String(zsum/zdatasize, 3));
      }
    }
  }

}

boolean APSetup() {
  digitalWrite(LED_BUILTIN, HIGH);
  Sprint("\n--Setting up Access Point "); Sprint(ssid); Sprintln(" ...");
  WiFi.setHostname("nano");
  WiFi.config(APIP, APIP, APIP, IPAddress(255,255,255,0));
  digitalWrite(LED_BUILTIN, LOW);
  APStatus = WiFi.beginAP(ssid, pass, APChannel);
  if (APStatus != WL_AP_LISTENING) {
    Sprint("\n--Setting up Access Point "); Sprint(ssid); Sprintln(" failed");
    return false;
  }
  webServer.begin();
  socketServer.begin();
  #ifdef DEBUG
  printWifiStatus();
  #endif
  WiFi.lowPowerMode();
  Sprint("--Access Point "); Sprint(ssid); Sprintln(" set up");
  digitalWrite(LED_BUILTIN, HIGH);
  return true;
}

void sendBase64Page(const char* base64Page, WiFiClient client, int packetSize) {
  const int base64Page_length = strlen(base64Page);
  const int page_length = base64_dec_len(base64Page, base64Page_length);
  char page[page_length];
  int done = 0;
  base64_decode(page, base64Page, base64Page_length);
  client.println("HTTP/1.1 200 OK\nConnection: close\nContent-Type: text/html");
  if (page[0] == 0x1f && page[1] == 0x8b) {
    client.println("Content-Encoding: gzip");
  }
  client.println("");
  while (page_length > done) {
    if (page_length - done < packetSize) {
      packetSize = page_length - done;
    }
    client.write(page + done, packetSize * sizeof(char));
    done = done + packetSize;
  }
}

#ifdef DEBUG
void printWifiStatus() {
  Sprint("Access point SSID: "); Sprintln(WiFi.SSID());
  Sprint("IP address: "); Sprintln(WiFi.localIP());
  Sprint("Gateway: "); Sprintln(WiFi.gatewayIP());
  Sprint("Netmask: "); Sprintln(WiFi.subnetMask());
  Sprint("Webserver is at http://"); Sprint(WiFi.localIP()); Sprint(":"); Sprint(webPort); Sprintln("/");
  Sprint("Websocket is at http://"); Sprint(WiFi.localIP()); Sprint(":"); Sprint(socketPort); Sprintln("/");
}
#endif
