#pragma once

#include <switch.h>

struct BluetoothDeviceInfo {
    BtdrvAddress address;
    char name[0xFF];
    u8 type;
    u8 rssi;
};

class BluetoothHandler {
public:
    BluetoothHandler();
    ~BluetoothHandler();
    
    bool initialize();
    bool startAdvertising();
    bool stopAdvertising();
    bool waitForConnection(u32 timeout_ms = 0); // 0 = бесконечное ожидание
    bool disconnect();
    bool sendKeyPress(u32 keyCode);
    bool isConnected();
    bool isAdvertising();
    
    // Новые методы для получения информации об устройстве
    bool getConnectedDeviceInfo(BluetoothDeviceInfo* deviceInfo);
    const char* getLastErrorMessage() const { return m_lastError; }

private:
    bool m_initialized;
    bool m_connected;
    bool m_advertising;
    BtdrvAddress m_deviceAddress;
    Event m_connectionEvent;
    BluetoothDeviceInfo m_connectedDevice;
    char m_lastError[256];
};
