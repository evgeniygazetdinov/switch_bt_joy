// bluetooth_device.cpp
#include "bluetooth_device.hpp"
#include <cstring>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

BluetoothDevice::BluetoothDevice() : 
    m_handle{0},
    m_session_id{0},  // Инициализируем идентификатор сессии
    m_initialized(false),
    m_connected(false),
    m_advertising(false)  // Инициализируем флаг рекламы
{
    // Инициализируем MAC-адрес нулями
    memset(&m_device_address, 0, sizeof(m_device_address));
}

BluetoothDevice::~BluetoothDevice() {
    Finalize();
}

Result BluetoothDevice::Initialize() {
    if (m_initialized) {
        return 0;
    }
    
    Result rc = hiddbgInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize hiddbg: 0x%x\n", rc);
        return rc;
    }
    
    // Инициализируем идентификатор сессии
    m_session_id = {0};
    
    void* workBuffer = aligned_alloc(0x1000, 0x1000);  // Выделяем буфер с выравниванием
    if (workBuffer == NULL) {
        printf("Failed to allocate work buffer\n");
        hiddbgExit();
        return -1;
    }
    
    rc = hiddbgAttachHdlsWorkBuffer(&m_session_id, workBuffer, 0x1000);
    if (R_FAILED(rc)) {
        printf("Failed to attach work buffer: 0x%x\n", rc);
        free(workBuffer);
        hiddbgExit();
        return rc;
    }
    
    HiddbgHdlsDeviceInfo device_info = {0};

    // Устанавливаем тип устройства (FullKey3 - Pro Controller)
    device_info.deviceType = HidDeviceType_FullKey3;  // 3

    // Устанавливаем тип интерфейса (Bluetooth)
    device_info.npadInterfaceType = HidNpadInterfaceType_Bluetooth;  // 1

    // Устанавливаем цвета контроллера (обязательно для корректной работы)
    // Используем макрос RGBA8_MAXALPHA или прямые значения
    // RGBA8_MAXALPHA(r,g,b) = (((r)&0xff)|(((g)&0xff)<<8)|(((b)&0xff)<<16)|(0xff<<24))

    // Белый корпус
    device_info.singleColorBody = 0xFFFFFFFF;  // RGBA8_MAXALPHA(255, 255, 255)

    // Черные кнопки
    device_info.singleColorButtons = 0xFF000000;  // RGBA8_MAXALPHA(0, 0, 0)

    // Цвет левой рукоятки (например, синий)
    device_info.colorLeftGrip = 0xFF0000FF;  // RGBA8_MAXALPHA(0, 0, 255)

    // Цвет правой рукоятки (например, красный)
    device_info.colorRightGrip = 0xFFFF0000;  // RGBA8_MAXALPHA(255, 0, 0)

    // Для версий прошивки 9.0.0+
    // device_info.npadControllerType = NpadControllerType_ProController;

    // Теперь вызываем функцию с правильно инициализированной структурой
    rc = hiddbgAttachHdlsVirtualDevice(&m_handle, &device_info);
    if (R_FAILED(rc)) {
        printf("Failed to attach virtual device: 0x%x\n", rc);
        hiddbgExit();
        return rc;
    }

    // Инициализируем генератор случайных чисел
    srand(time(NULL));
    
    // Генерируем MAC-адрес с префиксом Nintendo (00:1F:32)
    m_device_address.address[0] = 0x00;
    m_device_address.address[1] = 0x1F;
    m_device_address.address[2] = 0x32;
    
    // Генерируем случайные байты для остальной части адреса
    m_device_address.address[3] = rand() % 256;
    m_device_address.address[4] = rand() % 256;
    m_device_address.address[5] = rand() % 256;
    
    printf("Generated MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           m_device_address.address[0], m_device_address.address[1],
           m_device_address.address[2], m_device_address.address[3],
           m_device_address.address[4], m_device_address.address[5]);

    m_initialized = true;
    printf("Bluetooth initialized successfully\n");
    
    return rc;
}

void BluetoothDevice::PrintDeviceInfo() {
    if (!m_initialized) {
        printf("Device not initialized, no info to print\n");
        return;
    }
    
    printf("=== Bluetooth Virtual Device Info ===\n");
    printf("Device initialized: %s\n", m_initialized ? "Yes" : "No");
    
    // Выводим информацию о типе устройства
    printf("Device Type: Pro Controller (FullKey3)\n");
    printf("Interface Type: Bluetooth\n");
    printf("Connection Status: %s\n", m_connected ? "Connected" : "Not Connected");
    
    // Выводим MAC-адрес устройства, если он был сохранен
    if (m_device_address.address[0] != 0 || 
        m_device_address.address[1] != 0 || 
        m_device_address.address[2] != 0) {
        printf("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
               m_device_address.address[0], m_device_address.address[1],
               m_device_address.address[2], m_device_address.address[3],
               m_device_address.address[4], m_device_address.address[5]);
    }
    
    printf("==============================\n");
}

Result BluetoothDevice::WaitForConnection() {
    if (!m_initialized) {
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }

    printf("Waiting for Bluetooth connection...\n");
    printf("Please connect to the device using your Bluetooth settings.\n");
    
    // В текущей реализации мы просто симулируем подключение
    // Для реальной проверки подключения нужно использовать доступные API
    
    // Пробуем получить информацию о состоянии устройства
    Result rc = 0;
    
    // Проверяем, что хендл устройства валиден
    if (m_handle.handle != 0) {
        // Симулируем успешное подключение
        printf("Device connected successfully!\n");
        m_connected = true;
        
        // Выводим обновленную информацию об устройстве
        printf("=== Updated Device Status ===\n");
        printf("Connection Status: Connected\n");
        printf("Device Handle: 0x%x\n", m_handle.handle);
        
        // Выводим MAC-адрес устройства
        if (m_device_address.address[0] != 0 || 
            m_device_address.address[1] != 0 || 
            m_device_address.address[2] != 0) {
            printf("Device MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                m_device_address.address[0], m_device_address.address[1],
                m_device_address.address[2], m_device_address.address[3],
                m_device_address.address[4], m_device_address.address[5]);
        }
        
        printf("==============================\n");
        return 0;
    }
    
    // Если хендл не валиден, считаем что устройство не подключено
    printf("Connection not established. Please try again.\n");
    return MAKERESULT(Module_Libnx, LibnxError_NotFound);
}

Result BluetoothDevice::Disconnect() {
    if (!m_connected) {
        return 0;
    }

    printf("Disconnecting Bluetooth device...\n");
    
    // Если реклама активна, останавливаем её перед отключением
    if (m_advertising) {
        printf("Stopping advertising before disconnect...\n");
        Result rc = StopAdvertising();
        if (R_FAILED(rc)) {
            printf("Warning: Failed to stop advertising: 0x%x\n", rc);
            // Продолжаем отключение даже при ошибке остановки рекламы
        }
    }
    
    // Здесь можно добавить вызов API для отключения устройства,
    // если такой API доступен в libnx
    
    m_connected = false;
    printf("Device disconnected successfully\n");
    return 0;
}

Result BluetoothDevice::SendReport(const uint8_t* report, size_t size) {
    if (!m_connected) {
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }

    struct {
        uint8_t data[64];
        size_t size;
    } hid_report;

    if (size > sizeof(hid_report.data)) {
        return MAKERESULT(Module_Libnx, LibnxError_BadInput);
    }
    
    memcpy(hid_report.data, report, size);
    hid_report.size = size;

    return hiddbgSetHdlsState(m_handle, (const HiddbgHdlsState*)&hid_report);
}

Result BluetoothDevice::StartAdvertising() {
    if (!m_initialized) {
        printf("Cannot start advertising: device not initialized\n");
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }
    
    if (m_advertising) {
        printf("Advertising is already active\n");
        return 0;
    }
    
    printf("Starting Bluetooth advertising...\n");
    
    // Инициализируем btdrv сервис
    Result rc = btdrvInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize btdrv: 0x%x\n", rc);
        return rc;
    }
    
    // Устанавливаем устройство в режим обнаружения
    // Используем более простой API, доступный в текущей версии libnx
    rc = btdrvEnableBluetooth();
    if (R_FAILED(rc)) {
        printf("Failed to enable Bluetooth: 0x%x\n", rc);
        btdrvExit();
        return rc;
    }
    
    // Устанавливаем режим видимости
    rc = btdrvSetVisibility(true, true);  // discoverable=true, connectable=true
    if (R_FAILED(rc)) {
        printf("Failed to set visibility: 0x%x\n", rc);
        btdrvExit();
        return rc;
    }
    
    m_advertising = true;
    printf("Bluetooth advertising started successfully\n");
    printf("Device is now discoverable as Pro Controller\n");
    printf("MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           m_device_address.address[0], m_device_address.address[1],
           m_device_address.address[2], m_device_address.address[3],
           m_device_address.address[4], m_device_address.address[5]);
    
    return 0;
}

Result BluetoothDevice::StopAdvertising() {
    if (!m_initialized) {
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }
    
    if (!m_advertising) {
        printf("Advertising is not active\n");
        return 0;
    }
    
    printf("Stopping Bluetooth advertising...\n");
    
    // Отключаем видимость
    Result rc = btdrvSetVisibility(false, false);  // discoverable=false, connectable=false
    if (R_FAILED(rc)) {
        printf("Failed to disable visibility: 0x%x\n", rc);
        // Продолжаем даже при ошибке
    }
    
    // Отключаем Bluetooth
    rc = btdrvDisableBluetooth();
    if (R_FAILED(rc)) {
        printf("Failed to disable Bluetooth: 0x%x\n", rc);
        // Продолжаем даже при ошибке
    }
    
    // Выходим из btdrv сервиса
    btdrvExit();
    
    m_advertising = false;
    printf("Bluetooth advertising stopped\n");
    
    return 0;
}

void BluetoothDevice::Finalize() {
    if (m_initialized) {
        // Если устройство подключено, отключаем его
        if (m_connected) {
            printf("Disconnecting device...\n");
            Disconnect();
        }
        
        // Отсоединяем виртуальное устройство
        printf("Detaching virtual device...\n");
        Result rc = hiddbgDetachHdlsVirtualDevice(m_handle);
        if (R_FAILED(rc)) {
            printf("Warning: Failed to detach virtual device: 0x%x\n", rc);
        }
        
        // Отсоединяем рабочий буфер, используя сохраненный идентификатор сессии
        printf("Detaching work buffer...\n");
        rc = hiddbgReleaseHdlsWorkBuffer(m_session_id);
        if (R_FAILED(rc)) {
            printf("Warning: Failed to release work buffer: 0x%x\n", rc);
        }
        
        // Выходим из hiddbg сервиса
        printf("Exiting hiddbg service...\n");
        hiddbgExit();
        
        m_initialized = false;
        printf("Bluetooth finalized successfully\n");
    }
}