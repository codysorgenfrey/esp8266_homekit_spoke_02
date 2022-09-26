# ESP8266 Homekit Spoke 02
Control Mitsubishi heatpumps with homekit hub.

## Notes
No notes.

## Dependencies
[Arduino Websockets](https://github.com/Links2004/arduinoWebSockets)  
[Heatpump](https://github.com/SwiCago/HeatPump)  
[Arduino JSON](https://github.com/bblanchon/ArduinoJson)

## Required secrets.h
```
#pragma once
#ifndef __SECRETS_H__
#define __SECRETS_H__

#define WIFI_SSID ""
#define WIFI_PASS ""
#define SHEETS_URL ""
#define HP_SERIAL "" // Up stairs
// #define HP_SERIAL "" // Down stairs
#define WEBSOCKET_IP ""
#define WEBSOCKET_USER ""
#define WEBSOCKET_PASS ""

#endif
```