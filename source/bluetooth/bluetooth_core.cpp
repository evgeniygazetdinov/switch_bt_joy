#include "bluetooth_core.hpp"
#include <switch.h>

namespace bluetooth {
namespace core {

    namespace {
        bool g_initialized = false;
        bool g_enabled = false;
        BluetoothMode g_current_mode = BluetoothMode::None;
        
        os::SystemEvent g_system_event;
        os::SystemEvent g_forward_event;
    }

    Result Initialize() {
        if (g_initialized) {
            return 0;
        }

        Result rc = btdrvInitialize();
        if (R_FAILED(rc)) {
            return rc;
        }

        // Инициализируем системные события
        g_system_event.Initialize();
        g_forward_event.Initialize();

        g_initialized = true;
        return 0;
    }

    void Finalize() {
        if (!g_initialized) {
            return;
        }

        g_system_event.Finalize();
        g_forward_event.Finalize();

        btdrvExit();
        g_initialized = false;
        g_enabled = false;
        g_current_mode = BluetoothMode::None;
    }

    bool IsInitialized() {
        return g_initialized;
    }

    Result Enable() {
        if (!g_initialized) {
            return -1;
        }

        if (g_enabled) {
            return 0;
        }

        // Включаем Bluetooth и устанавливаем режим работы
        Result rc = btdrvEnable();
        if (R_FAILED(rc)) {
            return rc;
        }

        g_enabled = true;
        return 0;
    }

    Result Disable() {
        if (!g_initialized || !g_enabled) {
            return 0;
        }

        Result rc = btdrvDisable();
        if (R_FAILED(rc)) {
            return rc;
        }

        g_enabled = false;
        return 0;
    }

    bool IsEnabled() {
        return g_enabled;
    }

    Result SetMode(BluetoothMode mode) {
        if (!g_initialized || !g_enabled) {
            return -1;
        }

        BtdrvBluetoothMode btdrv_mode;
        switch (mode) {
            case BluetoothMode::None:
                btdrv_mode = BtdrvBluetoothMode_None;
                break;
            case BluetoothMode::Host:
                btdrv_mode = BtdrvBluetoothMode_Host;
                break;
            case BluetoothMode::Device:
                btdrv_mode = BtdrvBluetoothMode_Device;
                break;
            default:
                return -1;
        }
        Result rc = btdrvSetBluetoothMode(btdrv_mode);
        if (R_SUCCEEDED(rc)) {
            g_current_mode = mode;
        }

        return rc;
    }

    Result GetMode(BluetoothMode *out_mode) {
        if (!g_initialized || !out_mode) {
            return -1;
        }

        *out_mode = g_current_mode;
        return 0;
    }

    Result StartDiscovery() {
        if (!g_initialized || !g_enabled) {
            return -1;
        }

        return btdrvStartDiscovery();
    }

    Result CancelDiscovery() {
        if (!g_initialized || !g_enabled) {
            return -1;
        }

        return btdrvCancelDiscovery();
    }

    bool IsDiscovering() {
        if (!g_initialized || !g_enabled) {
            return false;
        }

        bool discovering = false;
        Result rc = btdrvIsDiscovering(&discovering);
        return R_SUCCEEDED(rc) && discovering;
    }

    Result GetDeviceInfo(const Address *address, DeviceInfo *out_device_info) {
        if (!g_initialized || !g_enabled || !address || !out_device_info) {
            return -1;
        }

        BtdrvDeviceProperty device_property;
        Result rc = btdrvGetDeviceProperty(address, &device_property);
        if (R_FAILED(rc)) {
            return rc;
        }

        out_device_info->address = *address;
        strncpy(out_device_info->name, device_property.name, sizeof(out_device_info->name) - 1);
        out_device_info->cod = device_property.cod;
        out_device_info->paired = device_property.paired;
        out_device_info->connected = device_property.connected;

        return 0;
    }

    Result Connect(const Address *address) {
        if (!g_initialized || !g_enabled || !address) {
            return -1;
        }

        return btdrvConnect(*address);
    }

    Result Disconnect(const Address *address) {
        if (!g_initialized || !g_enabled || !address) {
            return -1;
        }

        return btdrvCloseConnection(address);
    }

    Result RegisterEventCallback(EventType event_type, Event* event) {
        if (!g_initialized || !g_enabled || !event) {
            return -1;
        }

        return btdrvRegisterEventCallback(event_type, event);
    }

    Result UnregisterEventCallback(EventType event_type, Event* event) {
        if (!g_initialized || !g_enabled || !event) {
            return -1;
        }

        return btdrvUnregisterEventCallback(event_type, event);
    }

    Result GetEventInfo(EventType *out_type, void *out_buffer, size_t size) {
        if (!g_initialized || !g_enabled || !out_type || !out_buffer) {
            return -1;
        }

        return btdrvGetEventInfo(out_type, out_buffer, size);
    }

} // namespace core
} // namespace bluetooth
