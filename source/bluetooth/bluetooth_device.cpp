#include "bluetooth_device.hpp"
#include <switch.h>
#include <cstring>  // для memcpy

namespace {
    // HID дескриптор для геймпада
    constexpr uint8_t HID_DESCRIPTOR[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,        // Usage (Game Pad)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x05, 0x09,        //   Usage Page (Button)
        0x19, 0x01,        //   Usage Minimum (Button 1)
        0x29, 0x08,        //   Usage Maximum (Button 8)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x08,        //   Report Count (8)
        0x81, 0x02,        //   Input (Data,Var,Abs)
        0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
        0x09, 0x30,        //   Usage (X)
        0x09, 0x31,        //   Usage (Y)
        0x15, 0x81,        //   Logical Minimum (-127)
        0x25, 0x7F,        //   Logical Maximum (127)
        0x75, 0x08,        //   Report Size (8)
        0x95, 0x02,        //   Report Count (2)
        0x81, 0x02,        //   Input (Data,Var,Abs)
        0xC0              // End Collection
    };

    constexpr uint16_t VENDOR_ID = 0x0F0D;  // HORI
    constexpr uint16_t PRODUCT_ID = 0x0092; // HORIPAD
    constexpr uint16_t VERSION = 0x0100;    // v1.0
}

BluetoothDevice::BluetoothDevice()
    : m_initialized(false)
    , m_advertising(false)
    , m_connected(false) {
}

BluetoothDevice::~BluetoothDevice() {
    Finalize();
}

Result BluetoothDevice::Initialize() {
    if (m_initialized) {
        return 0;
    }

    Result rc;

    // Инициализируем Bluetooth стек
    rc = InitializeBluetooth();
    if (R_FAILED(rc)) {
        return rc;
    }

    // Включаем Bluetooth
    rc = EnableBluetooth();
    if (R_FAILED(rc)) {
        return rc;
    }

    // Настраиваем режим устройства
    rc = SetupDeviceMode();
    if (R_FAILED(rc)) {
        return rc;
    }

    // Настраиваем HID профиль
    rc = SetupHidProfile();
    if (R_FAILED(rc)) {
        return rc;
    }

    m_initialized = true;
    return 0;
}

void BluetoothDevice::Finalize() {
    if (!m_initialized) {
        return;
    }

    if (m_connected) {
        Disconnect();
    }

    if (m_advertising) {
        StopAdvertising();
    }

    btdrvFinalizeHid();
    m_initialized = false;
}

Result BluetoothDevice::InitializeBluetooth() {
    // Инициализируем HID стек с буфером для событий
    Result rc = btdrvInitializeHid(&m_event_buffer[0]);
    if (R_FAILED(rc)) {
        return rc;
    }

    // Ждем события инициализации
    while (true) {
        rc = eventWait(&m_event_buffer[0], UINT64_MAX);
        if (R_FAILED(rc)) {
            return rc;
        }

        // Событие получено, можно продолжать
        break;
    }

    return rc;
}

Result BluetoothDevice::EnableBluetooth() {
    // Включаем HID режим
    Result rc = btdrvEnableBle();
    if (R_FAILED(rc)) {
        return rc;
    }

    // Ждем события включения
    while (true) {
        rc = eventWait(&m_event_buffer[0], UINT64_MAX);
        if (R_FAILED(rc)) {
            return rc;
        }

        // Событие получено, можно продолжать
        break;
    }

    return rc;
}

Result BluetoothDevice::SetupDeviceMode() {
    // Настраиваем параметры BLE устройства
    BtdrvBleConnectionParameter param = {};
    param.min_conn_interval = 0x18;    // 30ms
    param.max_conn_interval = 0x28;    // 50ms
    param.scan_interval = 0x100;       // 160ms
    param.scan_window = 0x100;         // 160ms
    param.slave_latency = 0;
    param.supervision_tout = 0x1F4;    // 5000ms
    
    return btdrvSetBleDefaultConnectionParameter(&param);
}

Result BluetoothDevice::StartAdvertising() {
    if (!m_initialized || m_advertising) {
        return -1;
    }

    Result rc = StartAdvertising();
    if (R_SUCCEEDED(rc)) {
        m_advertising = true;
    }
    return rc;
}

Result BluetoothDevice::StopAdvertising() {
    if (!m_initialized || !m_advertising) {
        return -1;
    }

    Result rc = StopAdvertising();
    if (R_SUCCEEDED(rc)) {
        m_advertising = false;
    }
    return rc;
}

Result BluetoothDevice::WaitForConnection() {
    if (!m_initialized || !m_advertising || m_connected) {
        return -1;
    }

    // Ждем события подключения
    while (!m_connected) {
        Result rc = eventWait(&m_event_buffer[0], UINT64_MAX);
        if (R_FAILED(rc)) {
            return rc;
        }

        // Проверяем тип события
        BtdrvEventInfo event_info;
        BtdrvEventType event_type;
        rc = btdrvGetEventInfo(nullptr, &event_info, &event_type);
        if (R_FAILED(rc)) {
            return rc;
        }

        if (event_type == BtdrvEventType_Connection) {
            HandleConnectionEvent(&event_info);
        }
    }

    return 0;
}

Result BluetoothDevice::Disconnect() {
    if (!m_initialized || !m_connected) {
        return -1;
    }

    Result rc = btdrvCloseHidConnection(m_connected_address);
    if (R_SUCCEEDED(rc)) {
        m_connected = false;
    }
    return rc;
}

Result BluetoothDevice::SendReport(const uint8_t* report, size_t size) {
    if (!m_initialized || !m_connected) {
        return -1;
    }

    return btdrvSetHidReport(m_connected_address, 
                            BtdrvBluetoothHhReportType_Input,
                            report,
                            size);
}

void BluetoothDevice::HandleConnectionEvent(const void* event_data) {
    const EventInfo* info = static_cast<const EventInfo*>(event_data);
    m_connected_address = info->connection.address;
    m_connected = true;
}

void BluetoothDevice::HandleDisconnectionEvent(const void* event_data) {
    m_connected = false;
}

Result BluetoothDevice::SetupHidProfile() {
    /*
            BtdrvAddress - адрес устройства
        BtdrvBluetoothHhReportType - тип отчета (Input в нашем случае)
        const void* - указатель на данные
        size_t - размер данных 
    
    */
    return btdrvSetHidReport(m_connected_address, 
                            BtdrvBluetoothHhReportType_Input,
                            HID_DESCRIPTOR,
                            sizeof(HID_DESCRIPTOR));
}
