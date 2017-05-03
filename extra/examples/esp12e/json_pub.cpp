#ifndef UNIT_TEST

#include <robus.h>
#include <sys_msg.h>
#include <detection.h>

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
void reset_route_table();
unsigned char send_and_wait(msg_t msg);
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

typedef enum {
    IDENTIFY_CMD, // Gate asks a module to identify itself
    INTRODUCTION_CMD, // Module sends its alias and type to the gate

    ASK_PUB_CMD, // Gate asks a sensor module to publish its data
    PUBLISH_CMD, // Module publishes its data to the gate

    GATE_PROTOCOL_NB,
} module_register_t;


int i = 0;
int response = 0;
int listener = 0;
int nb_modules = 0;


vm_t *vm;

msg_t identify_msg;
msg_t publish_msg;


void rx_cb(msg_t *msg) {
  if (msg->header.cmd == INTRODUCTION_CMD) {
    char alias[MAX_ALIAS_SIZE];
    for (int i=0; i < MAX_ALIAS_SIZE; i++) {
      alias[i] = msg->data[i];
    }

    int id = msg->header.source;
    int type = msg->data[MAX_ALIAS_SIZE];

    route_table[id] = std::pair<int, String>(type, alias);
  }
  else if (msg->header.cmd == PUBLISH_CMD) {
    gathered_data[msg->header.source] = msg->data[0];
  }

  response = 1;
}


void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    #ifdef DEBUG
      delay(3000);
    #endif

    setup_wifi();

    robus_init();
    vm = robus_module_create(rx_cb, GATE_TYPE, "Larry Skywalker");

    identify_msg.header.cmd = IDENTIFY_CMD;
    identify_msg.header.target = BROADCAST_VAL;
    identify_msg.header.target_mode = BROADCAST;
    identify_msg.header.size = 1;

    publish_msg.header.cmd = ASK_PUB_CMD;
    publish_msg.header.target = BROADCAST_VAL;
    publish_msg.header.target_mode = BROADCAST;
    publish_msg.header.size = 1;
}


void loop() {
    // Restarts topology detection
    if (i % 10 == 0) {
      #ifdef DEBUG
        Serial.println("Launching topology detection");
      #endif
      reset_route_table();

      nb_modules = topology_detection(vm);
      #ifdef DEBUG
        Serial.printf("%d modules found.\n", nb_modules);
      #endif

      // Send identify msg
      for (int mod=2; mod<=nb_modules; mod++) {
        identify_msg.data[0] = mod;
        send_and_wait(identify_msg);
      }
      #ifdef DEBUG
        Serial.println("New route table:");
        for (auto const &el: route_table) {
          Serial.printf("alias \"%s\" - id %d - type %d\n",
                        el.second.second.c_str(), el.first, el.second.first);
        }
      #endif
    }

    webSocket.loop();

    // if (listener) {
      for (int mod=2; mod<=nb_modules; mod++) {
        publish_msg.data[0] = mod;
        send_and_wait(publish_msg);
      }
      publish_update();
    // }

    delay(100);
    i++;
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
    Serial.println(payload);
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
      listener = 0;
      break;

    case WStype_CONNECTED:
      #ifdef DEBUG
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      #endif
      listener = 1;
      break;

    case WStype_TEXT:
      // TODO: receive json, parse it and do stuff
      break;

    }
}

#define NB_TIMEOUT 100
#define WAIT_TIMEOUT (10)

unsigned char send_and_wait(msg_t msg) {
  response = 0;

  robus_send(vm, &msg);

  for (int i=0; i<NB_TIMEOUT; i++) {
    if (response) {
      return 0;
    }
    delay(WAIT_TIMEOUT);
  }
  return 1;
}

void reset_route_table() {
  route_table.clear();
  gathered_data.clear();
}

#endif /* UNIT_TEST */
