#pragma once

#include <switch.h>
#include <string>
// #include "bluetooth_handler.h"

class ConnectionLogger {
public:
    static bool initialize(const char* logPath);
    static bool logConnection(const BluetoothDeviceInfo& deviceInfo);
    static bool logDisconnection(const BluetoothDeviceInfo& deviceInfo);
    static const char* getLastError() { return s_lastError; }

private:
    static bool writeToLog(const char* message);
    static char s_logPath[FS_MAX_PATH];
    static char s_lastError[256];
    static bool s_initialized;
};
