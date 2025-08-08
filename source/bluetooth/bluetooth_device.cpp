// bluetooth_device.cpp
#include "bluetooth_device.hpp"
#include <cstring>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

BluetoothDevice::BluetoothDevice() : 
    m_handle{0},
    m_initialized(false),
    m_connected(false) 
{
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
    HiddbgHdlsSessionId sessionId = {0};
void* workBuffer = aligned_alloc(0x1000, 0x1000);  // Выделяем буфер с выравниванием
if (workBuffer == NULL) {
        printf("Failed to allocate work buffer\n");
        hiddbgExit();
        return -1;
    }
    
rc = hiddbgAttachHdlsWorkBuffer(&sessionId, workBuffer, 0x1000);
    if (R_FAILED(rc)) {
        printf("Failed to attach work buffer: 0x%x\n", rc);
    free(workBuffer);
        hiddbgExit();
        return rc;
    }
    // Создаем и инициализируем структуру
  // Полная инициализация структуры HiddbgHdlsDeviceInfo
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
    
    printf("==============================\n");
    printf("To connect to this device, search for Pro Controller in your Bluetooth settings.\n");
    printf("==============================\n");
}

Result BluetoothDevice::WaitForConnection() {
    if (!m_initialized) {
        return MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
    }

    m_connected = true;
    return 0;
}

Result BluetoothDevice::Disconnect() {
    if (!m_connected) {
        return 0;
    }

    m_connected = false;
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

void BluetoothDevice::Finalize() {
    if (m_initialized) {
        if (m_connected) {
            Disconnect();
        }
        hiddbgDetachHdlsVirtualDevice(m_handle);
        hiddbgExit();
        m_initialized = false;
    }
}