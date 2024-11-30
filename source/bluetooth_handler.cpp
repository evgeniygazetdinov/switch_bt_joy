#include "bluetooth_handler.h"
#include <string.h>
#include <stdio.h>

BluetoothHandler::BluetoothHandler() : 
    m_initialized(false), 
    m_connected(false),
    m_advertising(false) {
    memset(&m_deviceAddress, 0, sizeof(BtdrvAddress));
    memset(&m_connectedDevice, 0, sizeof(BluetoothDeviceInfo));
    memset(m_lastError, 0, sizeof(m_lastError));
    eventCreate(&m_connectionEvent, false);
}

BluetoothHandler::~BluetoothHandler() {
    if (m_connected) {
        disconnect();
    }
    if (m_advertising) {
        stopAdvertising();
    }
    if (m_initialized) {
        btdrvExit();
    }
    eventClose(&m_connectionEvent);
}

bool BluetoothHandler::initialize() {
    Result rc = btdrvInitialize();
    if (R_FAILED(rc)) {
        return false;
    }
    
    rc = btdrvEnableBluetooth();
    if (R_FAILED(rc)) {
        btdrvExit();
        return false;
    }

    // Установка режима Bluetooth
    rc = btmSetBluetoothMode(BtmBluetoothMode_StaticJoy);
    if (R_FAILED(rc)) {
        btdrvExit();
        return false;
    }
    
    m_initialized = true;
    return true;
}

bool BluetoothHandler::startAdvertising() {
    if (!m_initialized || m_advertising) {
        return false;
    }

    // Начинаем поиск устройств
    // timeout_ms = 10000 (10 секунд)
    // max_devices = 8
    Result rc = btdrvStartInquiry(8, 10000000000LL);
    if (R_FAILED(rc)) {
        return false;
    }

    m_advertising = true;
    return true;
}

bool BluetoothHandler::stopAdvertising() {
    if (!m_initialized || !m_advertising) {
        return false;
    }

    Result rc = btdrvStopInquiry();
    if (R_FAILED(rc)) {
        return false;
    }

    m_advertising = false;
    return true;
}

bool BluetoothHandler::waitForConnection(u32 timeout_ms) {
    if (!m_advertising || m_connected) {
        snprintf(m_lastError, sizeof(m_lastError), "Invalid state for connection");
        return false;
    }

    // Ожидаем события подключения
    Result rc = eventWait(&m_connectionEvent, timeout_ms * 1000000ULL);
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Connection timeout");
        return false;
    }

    // Временная переменная для адреса и настроек устройства
    
    BtmDeviceInfoV13 deviceInfo = {0};
    s32 device_count = 0;
    
    // Получаем информацию о подключенном устройстве
    rc = btmGetDeviceInfo(BtmProfile_Hid, &deviceInfo, sizeof(deviceInfo), &device_count);
    
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to get device info");
        return false;
    }
    BtdrvAddress tempAddress;
    m_connected = true;
    m_deviceAddress = tempAddress;
    m_connectedDevice.address = tempAddress;
    
    // Тип и RSSI пока установим по умолчанию, так как их нет в структуре
    m_connectedDevice.type = 0;  // HID device type
    m_connectedDevice.rssi = 0;  // Unknown signal strength
    
    // Копируем имя устройства
    strncpy(m_connectedDevice.name, (const char*)deviceInfo.name, sizeof(m_connectedDevice.name) - 1);
    m_connectedDevice.name[sizeof(m_connectedDevice.name) - 1] = '\0';
    
    stopAdvertising();
    return true;
}

bool BluetoothHandler::disconnect() {
    if (!m_connected) {
        return false;
    }
    
    Result rc = btdrvCloseHidConnection(m_deviceAddress);
    if (R_FAILED(rc)) {
        return false;
    }
    
    m_connected = false;
    return true;
}

bool BluetoothHandler::sendKeyPress(u32 keyCode) {
    if (!m_connected) {
        return false;
    }
    
    // Создаем пакет с данными нажатия клавиши
    BtdrvHidReport report;
    report.data[0] = keyCode;
    report.size = 8; // Стандартный размер HID отчета клавиатуры
    
    Result rc = btdrvSetHidReport(m_deviceAddress, 
                                 BtdrvBluetoothHhReportType_Input,
                                 &report);
    if (R_FAILED(rc)) {
        return false;
    }
    
    return true;
}

bool BluetoothHandler::isConnected() {
    return m_connected;
}

bool BluetoothHandler::isAdvertising() {
    return m_advertising;
}

bool BluetoothHandler::getConnectedDeviceInfo(BluetoothDeviceInfo* deviceInfo) {
    if (!m_connected || !deviceInfo) {
        snprintf(m_lastError, sizeof(m_lastError), "Not connected or invalid pointer");
        return false;
    }
    
    *deviceInfo = m_connectedDevice;
    return true;
}
