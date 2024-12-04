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

    constexpr uint8_t BLE_ADVERTISEMENT_DATA[] = {
        // Данные рекламы BLE
    };

    constexpr uint16_t VENDOR_ID = 0x0F0D;  // HORI
    constexpr uint16_t PRODUCT_ID = 0x0092; // HORIPAD
    constexpr uint16_t VERSION = 0x0100;    // v1.0
}

BluetoothDevice::BluetoothDevice() 
    : m_initialized(false)
    , m_connected(false)
    , m_waiting(false) {
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
    printf("btdrv initialized successfully\n");

    // Инициализируем BLE
    printf("Initializing BLE...\n");
    rc = btdrvInitializeBle(nullptr);  // Не используем event
    if (R_FAILED(rc)) {
        printf("Failed to initialize BLE: %x\n", rc);
        btdrvExit();
        return rc;
    }
    printf("BLE initialized successfully\n");

    // Включаем BLE режим
    printf("Enabling BLE...\n");
    rc = btdrvEnableBle();
    if (R_FAILED(rc)) {
        printf("Failed to enable BLE: %x\n", rc);
        btdrvFinalizeBle();
        btdrvExit();
        return rc;
    }
    printf("BLE enabled successfully\n");

    // Инициализируем HID стек
    printf("Initializing HID stack...\n");
    rc = btdrvInitializeHid(&m_event_buffer[0]);
    if (R_FAILED(rc)) {
        printf("Failed to initialize HID (error %x)\n", rc);
        btdrvFinalizeBle();
        btdrvExit();
        return rc;
    }
    printf("HID stack initialized successfully\n");

    // Настраиваем HID профиль
    printf("Setting up HID profile...\n");
    rc = SetupHidProfile();
    if (R_FAILED(rc)) {
        printf("Failed to setup HID profile: %x\n", rc);
        btdrvFinalizeHid();
        btdrvFinalizeBle();
        btdrvExit();
        return rc;
    }
    printf("HID profile setup successfully\n");

    // Делаем устройство видимым и доступным для подключения
    printf("Making device visible and connectable...\n");
    rc = btdrvSetBleVisibility(true, true);
    if (R_FAILED(rc)) {
        printf("Failed to set visibility: %x\n", rc);
        btdrvFinalizeHid();
        btdrvFinalizeBle();
        btdrvExit();
        return rc;
    }
    printf("Device visibility set successfully\n");

    // Настраиваем параметры рекламы
    printf("Setting up BLE advertising parameters...\n");
    BtdrvAddress addr = {0}; // Используем нулевой адрес, система сама назначит
    rc = btdrvSetBleAdvertiseParameter(addr, 0x0020, 0x0040); // Интервалы: 20ms - 40ms
    if (R_FAILED(rc)) {
        printf("Failed to set advertising parameters: %x\n", rc);
        btdrvFinalizeHid();
        btdrvFinalizeBle();
        btdrvExit();
        return rc;
    }
    printf("Advertising parameters set successfully\n");

    // Настраиваем данные рекламы
    printf("Setting up BLE advertising data...\n");
    BtdrvBleAdvertisePacketData adv_data = {0}; // Обнуляем всю структуру

    // Основные данные рекламы в unk_x6
    // Формат: [длина][тип][данные]
    const char* device_name = "Switch joystick";
    uint8_t name_len = strlen(device_name);

    // 1. Flags (3 байта)
    adv_data.unk_x6[0] = 2;    // Длина первого поля (flags)
    adv_data.unk_x6[1] = 0x01; // Тип: Flags
    adv_data.unk_x6[2] = 0x06; // Flags: LE General Discoverable + BR/EDR Not Supported

    // 2. UUID сервиса (4 байта)
    adv_data.unk_x6[3] = 3;    // Длина второго поля (UUID)
    adv_data.unk_x6[4] = 0x03; // Тип: Complete List of 16-bit Service UUIDs
    adv_data.unk_x6[5] = 0x12; // UUID: Human Interface Device (0x1812), младший байт
    adv_data.unk_x6[6] = 0x18; // UUID: Human Interface Device (0x1812), старший байт

    // 3. Имя устройства (длина + 2 байта)
    adv_data.unk_x6[7] = name_len + 1;  // Длина имени + 1 байт на тип
    adv_data.unk_x6[8] = 0x09;          // Тип: Complete Local Name
    memcpy(&adv_data.unk_x6[9], device_name, name_len);

    // Общий размер: flags(3) + uuid(4) + name_header(2) + name_len
    adv_data.size0 = 9 + name_len;

    // Дополнительные данные в unk_xA8 (если нужно)
    adv_data.size2 = 0;  // Пока не используем

    // Не используем entries
    adv_data.count = 0;

    printf("Advertising data size: %d bytes\n", adv_data.size0);
    printf("Data dump: ");
    for(int i = 0; i < adv_data.size0; i++) {
        printf("%02x ", adv_data.unk_x6[i]);
    }
    printf("\n");

    rc = btdrvSetBleAdvertiseData(&adv_data);
    if (R_FAILED(rc)) {
        printf("Failed to set advertising data: %x\n", rc);
        btdrvFinalizeHid();
        btdrvExit();
        return rc;
    }
    printf("Advertising data set successfully\n");



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

    btdrvFinalizeHid();
    btdrvExit();
    m_initialized = false;
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

Result BluetoothDevice::WaitForConnection() {
    if (!m_initialized || m_connected) {
        return -1;
    }

    // Проверяем события Bluetooth
    Result rc = eventWait(&m_event_buffer[0], 0); // Неблокирующий вызов
    if (R_FAILED(rc)) {
        if (rc == 0xEA01) { // Timeout - это нормально для неблокирующего режима
            return 0;
        }
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
        printf("Connected\n");
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
    printf("Setting up HID device profile...\n");
    
    // Настраиваем HID дескриптор
    BtdrvHidReport hid_descriptor = {};
    hid_descriptor.size = sizeof(HID_DESCRIPTOR);
    if (sizeof(HID_DESCRIPTOR) > sizeof(hid_descriptor.data)) {
        printf("HID descriptor too large: %zu > %zu\n", 
               sizeof(HID_DESCRIPTOR), sizeof(hid_descriptor.data));
        return -1;
    }
    memcpy(hid_descriptor.data, HID_DESCRIPTOR, sizeof(HID_DESCRIPTOR));

    // Регистрируем устройство как HID контроллер
    printf("Registering HID device...\n");
    Event regitster_event={0};
    Result rc = btdrvRegisterBleHidEvent(&regitster_event); // 0x02 = Gamepad
    if (R_FAILED(rc)) {
        printf("Failed to register HID device: %x\n", rc);
    } else {
        printf("Successfully registered HID device\n");
    }
    
    return rc;
}
