#include "bluetooth_device.hpp"
#include <switch.h>
#include <cstring>// для memcpy
#include <stdio.h> 

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
        printf("Device already initialized\n");
        return 0;
    }

    printf("Starting initialization...\n");
    Result rc = 0;

    // Инициализируем основной Bluetooth сервис
    printf("Initializing btdrv service...\n");
    rc = btdrvInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize btdrv: %x\n", rc);
        return rc;
    }

    // // Включаем Bluetooth
    // printf("Enabling Bluetooth...\n");
    // rc = btdrvEnableBluetooth();
    // if (R_FAILED(rc)) {
    //     printf("Failed to enable Bluetooth: %x\n", rc);
    //     btdrvExit();
    //     return rc;
    // }

    // // Даем время на включение Bluetooth
    // printf("Waiting for Bluetooth to initialize...\n");
    // svcSleepThread(1000000000ULL); // Ждем 1 секунду

    // Инициализируем HID стек
    printf("Initializing HID stack...\n");
    rc = btdrvInitializeHid(&m_event_buffer[0]);
    if (R_FAILED(rc)) {
        printf("Failed to initialize HID (error %x)\n", rc);
        btdrvDisableBluetooth();
        btdrvExit();
        return rc;
    }

    // Даем время на инициализацию HID
    svcSleepThread(1000000000ULL); // Ждем 1 секунду

    // Настраиваем HID профиль
    printf("Setting up HID profile...\n");
    rc = SetupHidProfile();
    if (R_FAILED(rc)) {
        printf("Failed to setup HID profile: %x\n", rc);
        btdrvFinalizeHid();
        btdrvDisableBluetooth();
        btdrvExit();
        return rc;
    }

    m_initialized = true;
    printf("Initialization completed successfully\n");
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
    printf("Starting Bluetooth initialization...\n");
    
    // Инициализируем основной Bluetooth сервис
    Result rc = btdrvInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize btdrv: %x\n", rc);
        return rc;
    }
    
    // Включаем Bluetooth
    // printf("Enabling Bluetooth...\n");
    // rc = btdrvEnableBluetooth();
    // if (R_FAILED(rc)) {
    //     printf("Failed to enable Bluetooth: %x\n", rc);
    //     btdrvExit();
    //     return rc;
    // }
    
    // Ждем включения Bluetooth
    // printf("Waiting for Bluetooth to initialize...\n");
    // svcSleepThread(1000000000ULL); // 1 секунда
    
    // Инициализируем HID стек
    // printf("Initializing HID stack...\n");
    // rc = btdrvInitializeHid(&m_event_buffer[0]);
    // if (R_FAILED(rc)) {
    //     printf("Failed to initialize HID (error %x). Make sure you have the required permissions.\n", rc);
    //     btdrvDisableBluetooth();
    //     btdrvExit();
    //     return rc;
    // }
    
    // // Даем время на инициализацию HID
    // svcSleepThread(1000000000ULL); // 1 секунда
    
    printf("Bluetooth initialization completed successfully\n");
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
        BtdrvEventInfo event_info = {};
        BtdrvEventType event_type;
        rc = btdrvGetEventInfo(&event_info, sizeof(event_info), &event_type);
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
    if (!m_connected) {
        return -1;
    }

    // Создаем HID репорт
    BtdrvHidReport hid_report = {};
    hid_report.size = size;
    if (size > sizeof(hid_report.data)) {
        return -1;
    }
    memcpy(hid_report.data, report, size);

    return btdrvSetHidReport(m_connected_address, 
                            BtdrvBluetoothHhReportType_Input,
                            &hid_report);
}

void BluetoothDevice::HandleConnectionEvent(const void* event_data) {
    const BtdrvEventInfo* info = static_cast<const BtdrvEventInfo*>(event_data);
    // В зависимости от версии прошивки, адрес находится в разных местах
    #if SWITCH_FIRMWARE >= MAKEFIRMVER(12,0,0)
        m_connected_address = info->inquiry_device.v12.addr;
    #else
        m_connected_address = info->inquiry_device.v1.addr;
    #endif
    m_connected = true;
}

void BluetoothDevice::HandleDisconnectionEvent(const void* event_data) {
    m_connected = false;
}

Result BluetoothDevice::SetupHidProfile() {
    // Создаем HID репорт с дескриптором
    BtdrvHidReport hid_descriptor = {};
    hid_descriptor.size = sizeof(HID_DESCRIPTOR);
    if (sizeof(HID_DESCRIPTOR) > sizeof(hid_descriptor.data)) {
        return -1;
    }
    memcpy(hid_descriptor.data, HID_DESCRIPTOR, sizeof(HID_DESCRIPTOR)); //копируем данные дискриптора в структуру

    return btdrvSetHidReport(m_connected_address, 
                            BtdrvBluetoothHhReportType_Feature,
                            &hid_descriptor);
}
