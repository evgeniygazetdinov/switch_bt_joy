#pragma once
#include <switch.h>

// Определяем версию прошивки
#define MAKEFIRMVER(major, minor, micro) ((major << 16) | (minor << 8) | (micro))
#define SWITCH_FIRMWARE MAKEFIRMVER(12,0,0)  // Используем последнюю версию

// Основные типы из libnx
using Address = ::BtdrvAddress;
using DeviceClass = ::BtdrvClassOfDevice;
using EventType = ::BtdrvEventType;
using EventInfo = ::BtdrvEventInfo;
using Event = ::Event;

class BluetoothDevice {
public:
    BluetoothDevice();
    ~BluetoothDevice();

    // Инициализация и завершение работы
    Result Initialize();
    void Finalize();
    bool IsInitialized() const { return m_initialized; }

    // Управление соединением
    Result WaitForConnection();
    Result Disconnect();
    bool IsConnected() const { return m_connected; }
    void StopWaiting() { m_waiting = false; }  

    // Отправка HID репорта
    Result SendReport(const uint8_t* report, size_t size);


private:
    // Методы для работы с Bluetooth
    Result EnableBluetooth();
    Result SetupDeviceMode();
    Result SetupHidProfile();

    // Обработчики событий
    void HandleConnectionEvent(const void* event_data);
    void HandleDisconnectionEvent(const void* event_data);

    // Буфер для событий Bluetooth
    Event m_event_buffer[0x400];
    bool m_initialized;
    bool m_connected;
    bool m_waiting;  
    Address m_connected_address;
};
