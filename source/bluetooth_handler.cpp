#include "bluetooth_handler.h"
#include "bluetooth/bluetooth_core.hpp"
#include <cstring>
#include <cstdio>

BluetoothHandler::BluetoothHandler() 
    : m_initialized(false)
    , m_connected(false)
    , m_advertising(false) {
    eventCreate(&m_connectionEvent, false);
}

BluetoothHandler::~BluetoothHandler() {
    if (m_initialized) {
        disconnect();
        bluetooth::core::Finalize();
    }
    eventClose(&m_connectionEvent);
}

bool BluetoothHandler::initialize() {
    if (m_initialized) {
        return true;
    }

    Result rc;
    
    // Инициализируем Bluetooth core
    rc = bluetooth::core::Initialize();
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to initialize Bluetooth core: 0x%x", rc);
        return false;
    }

    // Регистрируем callback для событий подключения
    rc = bluetooth::core::RegisterEventCallback(bluetooth::EventType_Connection, &m_connectionEvent);
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to register connection callback: 0x%x", rc);
        bluetooth::core::Finalize();
        return false;
    }

    m_initialized = true;
    return true;
}

bool BluetoothHandler::startAdvertising() {
    if (!m_initialized) {
        snprintf(m_lastError, sizeof(m_lastError), "Bluetooth not initialized");
        return false;
    }

    if (m_advertising) {
        return true;
    }

    Result rc = bluetooth::core::StartDiscovery();
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to start advertising: 0x%x", rc);
        return false;
    }

    m_advertising = true;
    return true;
}

bool BluetoothHandler::stopAdvertising() {
    if (!m_initialized || !m_advertising) {
        return true;
    }

    Result rc = bluetooth::core::CancelDiscovery();
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to stop advertising: 0x%x", rc);
        return false;
    }

    m_advertising = false;
    return true;
}

bool BluetoothHandler::waitForConnection(u32 timeout_ms) {
    if (!m_initialized || m_connected) {
        return false;
    }

    // Ждем события подключения
    if (!eventWait(&m_connectionEvent, timeout_ms * 1000000ULL)) {
        return false;
    }

    // Получаем информацию о событии
    bluetooth::EventType type;
    bluetooth::EventInfo info;
    Result rc = bluetooth::core::GetEventInfo(&type, &info, sizeof(info));
    if (R_FAILED(rc)) {
        return false;
    }

    if (type == bluetooth::EventType_Connection) {
        // Получаем адрес подключенного устройства из структуры события
        BtdrvEventInfo* btdrv_info = reinterpret_cast<BtdrvEventInfo*>(&info);
        m_deviceAddress = btdrv_info->connection.addr;
        m_connected = true;
        return true;
    }

    return false;
}

bool BluetoothHandler::disconnect() {
    if (!m_initialized || !m_connected) {
        return true;
    }

    Result rc = bluetooth::core::Disconnect(&m_deviceAddress);
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to disconnect: 0x%x", rc);
        return false;
    }

    m_connected = false;
    return true;
}

bool BluetoothHandler::sendKeyPress(u32 keyCode) {
    if (!m_initialized || !m_connected) {
        return false;
    }

    // Формируем HID отчет
    bluetooth::HidReport report = {};
    report.data[0] = keyCode & 0xFF;
    report.size = 1;

    Result rc = btdrvSetHidReport(m_deviceAddress,
                                  bluetooth::HhReportType_Input,
                                  &report);
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to send HID report: 0x%x", rc);
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

bool BluetoothHandler::getConnectedDeviceInfo(bluetooth::DeviceInfo* deviceInfo) {
    if (!m_initialized || !m_connected || !deviceInfo) {
        return false;
    }

    Result rc = bluetooth::core::GetDeviceInfo(&m_deviceAddress, deviceInfo);
    if (R_FAILED(rc)) {
        snprintf(m_lastError, sizeof(m_lastError), "Failed to get device info: 0x%x", rc);
        return false;
    }

    return true;
}
