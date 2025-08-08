// bluetooth_device.hpp
#ifndef BLUETOOTH_DEVICE_HPP
#define BLUETOOTH_DEVICE_HPP

#include <switch.h>

class BluetoothDevice {
private:
    HiddbgHdlsHandle m_handle;
    HiddbgHdlsSessionId m_session_id;  // Session ID
    bool m_initialized;
    bool m_connected;
    bool m_advertising;  // Flag to track advertising state
    BtdrvAddress m_device_address;  // Device MAC address

    void Finalize();

public:
    BluetoothDevice();
    ~BluetoothDevice();
    void PrintDeviceInfo();
    Result Initialize();
    Result StartAdvertising();  // New method to start Bluetooth advertising
    Result StopAdvertising();   // New method to stop Bluetooth advertising
    Result WaitForConnection();
    Result Disconnect();
    Result SendReport(const uint8_t* report, size_t size);
    bool IsConnected() const { return m_connected; }
    bool IsAdvertising() const { return m_advertising; }  // Getter for advertising state
    
    // Public method for explicit Finalize() call
    void Shutdown() { Finalize(); }
};

#endif // BLUETOOTH_DEVICE_HPP