#ifndef UNIT_TEST

#include <robus.h>

#include <Arduino.h>
#include <Hash.h>

#include <string>
#include <map>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsServer.h>

#define CONNECTION_TIMEOUT 5000 // in ms

#define HOTSPOT_SSID "demo-pollen-robotics"
#define HOTSPOT_PASSWD "robuscesttropcool"

#define HOSTNAME "demo-pollen-robotics"

// TODO: check si ça marche
#define MDNS_SERVICE_NAME "jsongate"
#define MDNS_SERVICE_PORT 9342

typedef struct {
  const char *ssid;
  const char *password;
} wifi_t;

const wifi_t wifi_conf[] = {
  {"Pollen-Robotics", "chezpollencesttropcool"},
  {"BlackBerry", "clefwpafacile"},
};

ESP8266WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(MDNS_SERVICE_PORT);

void setup_wifi();
void publish_update();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

// route_table[id] = (type, alias)
std::map<int, std::pair<int, String> >route_table;
// data[id] = val
std::map<int, int>gathered_data;

typedef enum {
  GATE_TYPE,
  MOTOR_TYPE,
  LED_TYPE,
  POTARD_TYPE,
  BUTTON_TYPE,
} module_type_t;


int listner = 0;
vm_t *vm;


void rx_cb(msg_t *msg) {
  // TODO: setup introduce/publish cmd to gather data
}


void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    #ifdef DEBUG
      delay(3000);
    #endif

    setup_wifi();

    // Fill up with fake data.
    route_table[12] = std::pair<int, String>(BUTTON_TYPE, "Butt");
    route_table[21] = std::pair<int, String>(LED_TYPE, "LED");
    route_table[32] = std::pair<int, String>(MOTOR_TYPE, "Motor");
    gathered_data[12] = 78;

    robus_init();
    vm = robus_module_create(rx_cb, 1, "Larry Skywalker");
    vm->id = 42;

    // TODO: Launch topology detection.
}


void loop() {
    webSocket.loop();

    if (listner) {
      publish_update();
    }

    delay(100);
}


String type_from_enum(module_type_t type) {
  if (type == BUTTON_TYPE) {
    return "button";

  } else if (type == LED_TYPE) {
    return "led";

  } else if (type == MOTOR_TYPE) {
    return "motor";
  }
  return "unknown";
}

void publish_update() {
  String payload = "";

  payload += "{";
  payload += "\"modules\": [";

  for (auto const &el: route_table) {
    payload += "{";

    payload += "\"type\": \"" + type_from_enum((module_type_t)el.second.first) + "\", ";
    payload += "\"id\": " + String(el.first) + ", ";
    payload += "\"alias\": \"" + el.second.second + "\"";

    if (el.second.first == BUTTON_TYPE) {
      payload += ", \"value\": " + String(gathered_data[el.first]);
    }

    payload += "}, ";
  }
  if (route_table.size() > 0) {
    // HACK: remove last " ,"
    payload.remove(payload.length() - 2);
  }

  payload += "]";
  payload += "}";

  webSocket.sendTXT(0, payload);
  #ifdef DEBUG
    // Serial.println(payload);
  #endif
}

void setup_wifi() {
  for (int i=0; i<(sizeof(wifi_conf)/sizeof(wifi_t)); i++) {
    wifi_t conf = wifi_conf[i];
    WiFiMulti.addAP(conf.ssid, conf.password);
  }

  // First, try to connect to a known wifi.
  for (int i=0; i<10; i++) {
    if (WiFiMulti.run() == WL_CONNECTED) {
      break;
    }
    delay(CONNECTION_TIMEOUT / 10);
  }
  if (WiFiMulti.run() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    #ifdef DEBUG
      Serial.println("Connected to the Wifi.");
      Serial.print("IP address: ");
      Serial.println(ip);
    #endif
  }
  // If it fails, we create our hotspot.
  else {
    WiFi.softAP(HOTSPOT_SSID, HOTSPOT_PASSWD);
    IPAddress ip = WiFi.softAPIP();
    #ifdef DEBUG
      Serial.println("Could not connect to the Wifi.");
      Serial.println("Create our own hotspot");
      Serial.print("IP address: ");
      Serial.println(ip);
    #endif
  }

  // Setup mDNS
  WiFi.hostname(HOSTNAME);
  MDNS.begin(HOSTNAME);
  MDNS.addService(MDNS_SERVICE_NAME, "tcp", MDNS_SERVICE_PORT);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      #ifdef DEBUG
        Serial.printf("[%u] Disconnected!\n", num);
      #endif
      listner = 0;
      break;

    case WStype_CONNECTED:
      #ifdef DEBUG
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      #endif
      listner = 1;
      break;

    case WStype_TEXT:
      // TODO: receive json, parse it and do stuff
      break;

    }
}

#endif /* UNIT_TEST */
