#include "common.h"
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <HeatPump.h>

WebSocketsClient webSocket;
HeatPump hp;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			HK_INFO_LINE("Websocket disconnected.");
			break;
		case WStype_CONNECTED:
			HK_INFO_LINE("Websocket connected to url: %s", payload);
			webSocket.sendTXT("Connected");
			break;
		case WStype_TEXT:
			HK_INFO_LINE("Websocket got text: %s", payload);
			break;
		case WStype_BIN:
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}
}

void heatpumpSettingsChanged() {

}

void heatpumpStatusChanged(heatpumpStatus status) {

}

void heatpumpRoomTempChanged() {

}

void setup() {
	#if HK_DEBUG > HK_DEBUG_LEVEL_NONE
        Serial.begin(115200);
        while (!Serial) { ; } // wait for serial
    #endif

    HK_INFO_LINE("Connecting to " WIFI_SSID ".");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    HK_INFO_LINE("Connected.");

    HK_INFO_LINE("Connecting to homekit hub.");
    webSocket.begin(WEBSOCKET_IP, 81);
    webSocket.setAuthorization(WEBSOCKET_USER, WEBSOCKET_PASS);
    webSocket.setReconnectInterval(5000);
    webSocket.onEvent(webSocketEvent);

    #if HP_CONNECTED
        if (hp.connect(&Serial)) {
            hp.enableExternalUpdate(); // implies autoUpdate as well
            hp.setSettingsChangedCallback(heatpumpSettingsChanged);
            hp.setStatusChangedCallback(heatpumpStatusChanged);
            hp.setRoomTempChangedCallback(heatpumpRoomTempChanged);
        } else HK_ERROR_LINE("Error connecting to heatpump.");
    #else
        HK_INFO_LINE("Skipping heatpump setup, not connected.");
    #endif
}

void loop() {
	webSocket.loop();
}
