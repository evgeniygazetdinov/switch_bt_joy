// bluetooth_device.hpp
#ifndef BLUETOOTH_DEVICE_HPP
#define BLUETOOTH_DEVICE_HPP

#include <switch.h>

class BluetoothDevice {
private:
    HiddbgHdlsHandle m_handle;
    bool m_initialized;
    bool m_connected;

    void Finalize();

public:
    BluetoothDevice();
    ~BluetoothDevice();
    void PrintDeviceInfo();
    Result Initialize();
    Result WaitForConnection();
    Result Disconnect();
    Result SendReport(const uint8_t* report, size_t size);
    bool IsConnected() const { return m_connected; }
};

#endif // BLUETOOTH_DEVICE_HPP