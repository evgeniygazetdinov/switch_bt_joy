#pragma once
#include <switch.h>

// Основные типы из libnx
using Address = ::BtdrvAddress;
using DeviceClass = ::BtdrvClassOfDevice;
using EventType = ::BtdrvEventType;
using EventInfo = ::BtdrvEventInfo;

class BluetoothDevice {
public:
    BluetoothDevice();
    ~BluetoothDevice();

    // Инициализация и завершение работы
    Result Initialize();
    void Finalize();
    bool IsInitialized() const { return m_initialized; }

    // Управление соединением
    Result StartAdvertising();
    Result StopAdvertising();
    bool IsAdvertising() const { return m_advertising; }
    Result WaitForConnection();
    Result Disconnect();
    bool IsConnected() const { return m_connected; }

    // Отправка HID репорта
    Result SendReport(const uint8_t* report, size_t size);

private:
    bool m_initialized;
    bool m_advertising;
    bool m_connected;
    Address m_connected_address;
    
    // Буфер для событий Bluetooth
    Event m_event_buffer[0x400];

    // Инициализация Bluetooth стека
    Result InitializeBluetooth();
    Result EnableBluetooth();
    Result SetupDeviceMode();

    // Настройка HID профиля
    Result SetupHidProfile();
    Result RegisterHidEvents();
    
    // Обработка событий
    void HandleConnectionEvent(const void* event_data);
    void HandleDisconnectionEvent(const void* event_data);
};
