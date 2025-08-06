// bluetooth_device.cpp
#include "bluetooth_device.hpp"
#include <cstring>
#include <stdio.h>

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

    // Создаем и инициализируем структуру
    HiddbgHdlsDeviceInfo device_info = {0};

    // Устанавливаем тип устройства (FullKey3 - Pro Controller)
    device_info.deviceType = HidDeviceType_FullKey3;

    // Устанавливаем тип интерфейса (Bluetooth)
    device_info.npadInterfaceType = HidNpadInterfaceType_Bluetooth;

    // Устанавливаем цвета контроллера
    device_info.singleColorBody = RGBA8_MAXALPHA(255, 255, 255);      // Белый корпус
    device_info.singleColorButtons = RGBA8_MAXALPHA(0, 0, 0);         // Черные кнопки
    device_info.colorLeftGrip = RGBA8_MAXALPHA(230, 255, 0);          // Цвет левой рукоятки
    device_info.colorRightGrip = RGBA8_MAXALPHA(0, 40, 20);           // Цвет правой рукоятки

    // Дополнительно для [9.0.0+] можно установить тип контроллера Npad
    // device_info.npadControllerType = NpadControllerType_ProController;

    // Теперь вызываем функцию с правильно инициализированной структурой
    rc = hiddbgAttachHdlsVirtualDevice(&m_handle, &device_info);

    if (R_FAILED(rc)) {
        printf("Failed to attach virtual device: 0x%x\n", rc);
        hiddbgExit();
        return rc;
    }

    m_initialized = true;
    return rc;
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