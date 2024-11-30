#pragma once

#include <switch.h>
#include "bluetooth/bluetooth_types.hpp"

// Максимальные размеры буферов
#define BT_MAX_ERROR_MSG 256

/**
 * @struct BluetoothDeviceInfo
 * @brief Информация о Bluetooth устройстве
 */
//struct BluetoothDeviceInfo {
//    BtdbAddress address;              ///< MAC-адрес устройства
//    char name[BT_MAX_DEVICE_NAME];    ///< Имя устройства
//    u8 type;                         ///< Тип устройства
//    s8 rssi;                         ///< Уровень сигнала (dBm)
//    bool is_connected;               ///< Статус подключения
//};

class BluetoothHandler {
public:
    BluetoothHandler();
    ~BluetoothHandler();
    
    bool initialize();
    bool startAdvertising();
    bool stopAdvertising();
    bool waitForConnection(u32 timeout_ms); // 0 = бесконечное ожидание
    bool disconnect();
    bool sendKeyPress(u32 keyCode);
    bool isConnected();
    bool isAdvertising();
    
    // Новые методы для получения информации об устройстве
    bool getConnectedDeviceInfo(bluetooth::DeviceInfo* deviceInfo);
    const char* getLastErrorMessage() const { return m_lastError; }

private:
    bool m_initialized;
    bool m_connected;
    bool m_advertising;
    bluetooth::Address m_deviceAddress;
    Event m_connectionEvent;
    bluetooth::DeviceInfo m_connectedDevice;
    char m_lastError[BT_MAX_ERROR_MSG];
};
