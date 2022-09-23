#include "common.h"
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <HeatPump.h>

WebSocketsClient webSocket;
HeatPump hp;
unsigned long disconnectedTimer;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			HK_INFO_LINE("Websocket disconnected.");
            disconnectedTimer = millis();
			break;
		case WStype_CONNECTED:
			HK_INFO_LINE("Websocket connected to url: %s", payload);
			webSocket.sendTXT("Connected");
            disconnectedTimer = 0;
			break;
		case WStype_TEXT: {
			HK_INFO_LINE("Websocket got text: %s", payload);
            String strPayload((char *)payload);
            if (
                strPayload != String("Connected") &&
                strPayload != String("Error") &&
                strPayload != String("Success")
            ) {
                StaticJsonDocument<192> doc;
                DeserializationError err = deserializeJson(doc, payload);
                if (err) HK_ERROR_LINE("Error deserializing message: %s", payload);
                else {
                    if (
                        doc["device"].as<String>() == String(HP_SERIAL) && 
                        doc["command"].as<String>() == String("update_settings")
                    ) {
                        #if HP_CONNECTED
                            heatpumpSettings settings = hp.getSettings();
                            settings.power = doc["payload"]["power"].as<const char *>();
                            settings.mode = doc["payload"]["mode"].as<const char *>();
                            settings.temperature = doc["payload"]["temperature"].as<float>();
                            settings.fan = doc["payload"]["fan"].as<const char *>(),
                            settings.vane = doc["payload"]["vane"].as<const char *>();
                            settings.wideVane = doc["payload"]["wideVane"].as<const char *>();

                            hp.setSettings(settings);
                            hp.update();
                        #endif
                        webSocket.sendTXT("Success");
                        HK_INFO_LINE("Updated heatpump settings.");
                    }
                }

                doc.clear();
            }
			break;
        }
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
    HK_INFO_LINE("Got new settings from heatpump.");
    heatpumpSettings settings = hp.getSettings();

    StaticJsonDocument<192> doc;
    doc["device"] = HP_SERIAL;
    doc["command"] = "update_settings";
    doc["payload"]["power"] = settings.power;
    doc["payload"]["mode"] = settings.mode;
    doc["payload"]["temperature"] = settings.temperature;
    doc["payload"]["fan"] = settings.fan;
    doc["payload"]["vane"] = settings.vane;
    doc["payload"]["wideVane"] = settings.wideVane;
    doc["payload"]["connected"] = settings.connected;
    String message;
    serializeJson(doc, message);

    if (!webSocket.sendTXT(message)) 
        HK_ERROR_LINE("Failed to send heatpump settings to hub. %s", message.c_str());

    doc.clear();
}

void heatpumpStatusChanged(heatpumpStatus status) {
    HK_INFO_LINE("Got new status from heatpump.");

    StaticJsonDocument<192> doc;
    doc["device"] = HP_SERIAL;
    doc["command"] = "update_status";
    doc["payload"]["roomTemperature"] = status.roomTemperature;
    doc["payload"]["operating"] = status.operating;
    doc["payload"]["compressorFrequency"] = status.compressorFrequency;
    String message;
    serializeJson(doc, message);

    if (!webSocket.sendTXT(message)) 
        HK_ERROR_LINE("Failed to send heatpump status to hub. %s", message.c_str());

    doc.clear();
}

void setup() {
	#if HK_DEBUG > HK_DEBUG_LEVEL_NONE
        Serial.begin(115200);
        while (!Serial) { ; } // wait for serial
    #else 
        pinMode(TX_PIN, OUTPUT); 
        pinMode(RX_PIN, INPUT_PULLUP);
    #endif

    HK_INFO_LINE("Connecting to " WIFI_SSID ".");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    HK_INFO_LINE("Connected. IP: %s", WiFi.localIP().toString().c_str());

    sl_printf(SHEETS_URL, "Homkit Spoke " HP_SERIAL, "Rebooting...");

    HK_INFO_LINE("Connecting to homek it hub.");
    webSocket.begin(WEBSOCKET_IP, 81);
    webSocket.setAuthorization(WEBSOCKET_USER, WEBSOCKET_PASS);
    webSocket.setReconnectInterval(5000);
    webSocket.onEvent(webSocketEvent);

    #if HP_CONNECTED
        hp.enableExternalUpdate(); // implies autoUpdate as well
        hp.setSettingsChangedCallback(heatpumpSettingsChanged);
        hp.setStatusChangedCallback(heatpumpStatusChanged);
        if (!hp.connect(&Serial)) HK_ERROR_LINE("Error connecting to heatpump.");
    #else
        HK_INFO_LINE("Skipping heatpump setup, not connected.");
    #endif
}

void loop() {
	webSocket.loop();

    #if HP_CONNECTED
        hp.sync();
    #endif 

    if (!webSocket.isConnected()) {
        unsigned long diff = max(millis(), disconnectedTimer) - min(millis(), disconnectedTimer);
        if (millis() % 5000 == 0) HK_INFO_LINE("Disconnected from hub for %.2f seconds.", diff / 1000.0f);
        if (diff >= MAX_DISCONNECT_TIME) 
            HK_ERROR_LINE("Homekit spoke " HP_SERIAL " cannot connect to hub.");
    }
}
