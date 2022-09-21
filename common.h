#pragma once
#ifndef __COMMON_H__
#define __COMMON_H__

#include "secrets.h"
#include <SheetsLogger.h>

// Logging
#define HK_DEBUG_LEVEL_NONE -1
#define HK_DEBUG_LEVEL_ERROR 0
#define HK_DEBUG_LEVEL_INFO 1

#define HK_DEBUG HK_DEBUG_LEVEL_INFO
#define HP_CONNECTED false

#if HK_DEBUG >= HK_DEBUG_LEVEL_ERROR
    #if !HP_CONNECTED
        #define HK_ERROR_LINE(message, ...) sl_printf(SHEETS_URL, "Homekit Spoke 02", "ERR [%7lu][%.2fkb] %s: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), HP_SERIAL, ##__VA_ARGS__)
    #else 
        #define HK_ERROR_LINE(message, ...) sl_printCloud(SHEETS_URL, "Homekit Spoke 02", "ERR [%7lu][%.2fkb] %s: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), HP_SERIAL, ##__VA_ARGS__)
    #endif
#else
    #define HK_ERROR_LINE(message, ...)
#endif

#if HK_DEBUG >= HK_DEBUG_LEVEL_INFO && !HP_CONNECTED
    #define HK_INFO_LINE(message, ...) printf(">>> [%7lu][%.2fkb] %s: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), HP_SERIAL, ##__VA_ARGS__)
#else
    #define HK_INFO_LINE(message, ...)
#endif

#endif