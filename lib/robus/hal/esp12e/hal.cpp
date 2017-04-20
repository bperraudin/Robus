#include "hal.h"

#include "context.h"

#ifdef ESP_USE_WIFI
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

const char* ssid = "***";
const char* password = "***";

const char* broker_host = "***";
const int broker_port = 9010;

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

#endif


void hal_init(void) {

#ifdef ESP_USE_WIFI
  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED) {
      delay(500);
  }
#ifdef DEBUG
  Serial.println("Connected to the Wifi.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  if (!client.connect(broker_host, broker_port)) {
      Serial.println("Connection to the broker failed.");
      return;
  }
#endif

}

unsigned char hal_transmit(unsigned char *data, unsigned short size) {

#ifdef ESP_USE_WIFI
    client.write((uint8_t *)data, (size_t)size);
#endif

    return 0;
}


void hal_loop() {

#ifdef ESP_USE_WIFI
    unsigned char c;
    while (client.available()) {
        c = (unsigned char)client.read();
        ctx.data_cb(&c);
    }
#endif

}
