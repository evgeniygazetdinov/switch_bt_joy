// bluetooth_device.hpp
#ifndef BLUETOOTH_DEVICE_HPP
#define BLUETOOTH_DEVICE_HPP

#include <switch.h>

class BluetoothDevice {
private:
    HiddbgHdlsHandle m_handle;
    HiddbgHdlsSessionId m_session_id;  // Добавляем идентификатор сессии
    bool m_initialized;
    bool m_connected;
    bool m_advertising;  // Флаг для отслеживания состояния рекламы
    BtdrvAddress m_device_address;  // MAC-адрес устройства

    void Finalize();

public:
    BluetoothDevice();
    ~BluetoothDevice();
    void PrintDeviceInfo();
    Result Initialize();
    Result StartAdvertising();  // Новый метод для запуска Bluetooth-рекламы
    Result StopAdvertising();   // Новый метод для остановки Bluetooth-рекламы
    Result WaitForConnection();
    Result Disconnect();
    Result SendReport(const uint8_t* report, size_t size);
    bool IsConnected() const { return m_connected; }
    bool IsAdvertising() const { return m_advertising; }  // Геттер для состояния рекламы
    
    // Публичный метод для явного вызова Finalize()
    void Shutdown() { Finalize(); }
};

#endif // BLUETOOTH_DEVICE_HPP