#include <cstring>
#include <cstdio>
#include <ctime>
#include "connection_logger.h"

char ConnectionLogger::s_logPath[256] = {0};
char ConnectionLogger::s_lastError[256] = {0};
bool ConnectionLogger::s_initialized = false;

bool ConnectionLogger::initialize(const char* logPath) {
    if (s_initialized) {
        return false;
    }
    
    if (!logPath) {
        snprintf(s_lastError, sizeof(s_lastError), "Invalid log path");
        return false;
    }

    Result rc = fsdevMountSdmc();
    if (R_FAILED(rc)) {
        snprintf(s_lastError, sizeof(s_lastError), "Failed to mount SD card");
        return false;
    }

    strncpy(s_logPath, logPath, sizeof(s_logPath) - 1);
    s_logPath[sizeof(s_logPath) - 1] = '\0';
    
    // Создаем или очищаем файл лога
    FILE* fp = fopen(s_logPath, "w");
    if (!fp) {
        snprintf(s_lastError, sizeof(s_lastError), "Failed to open log file");
        return false;
    }
    
    time_t now = time(NULL);
    fprintf(fp, "\n--- New Session Started at %s---\n", ctime(&now));
    fprintf(fp, "Timestamp,Event,DeviceName,DeviceAddress,RSSI\n");
    fclose(fp);
    
    s_initialized = true;
    return true;
}

bool ConnectionLogger::writeToLog(const char* message) {
    if (!s_initialized) {
        snprintf(s_lastError, sizeof(s_lastError), "Logger not initialized");
        return false;
    }

    FILE* file = fopen(s_logPath, "a");
    if (!file) {
        snprintf(s_lastError, sizeof(s_lastError), "Failed to open log file");
        return false;
    }

    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    fprintf(file, "[%s] %s\n", timestamp, message);
    fclose(file);

    return true;
}

bool ConnectionLogger::logConnection(const BluetoothDeviceInfo& deviceInfo) {
    char message[512];
    snprintf(message, sizeof(message),
             "Device Connected - Name: %s, Address: %02X:%02X:%02X:%02X:%02X:%02X, RSSI: %d dBm",
             deviceInfo.name,
             deviceInfo.address.address[0], deviceInfo.address.address[1],
             deviceInfo.address.address[2], deviceInfo.address.address[3],
             deviceInfo.address.address[4], deviceInfo.address.address[5],
             deviceInfo.rssi);

    return writeToLog(message);
}

bool ConnectionLogger::logDisconnection(const BluetoothDeviceInfo& deviceInfo) {
    char message[512];
    snprintf(message, sizeof(message),
             "Device Disconnected - Name: %s, Address: %02X:%02X:%02X:%02X:%02X:%02X",
             deviceInfo.name,
             deviceInfo.address.address[0], deviceInfo.address.address[1],
             deviceInfo.address.address[2], deviceInfo.address.address[3],
             deviceInfo.address.address[4], deviceInfo.address.address[5]);

    return writeToLog(message);
}
