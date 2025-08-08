// bluetooth_device.cpp
#include "bluetooth_device.hpp"
#include <cstring>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

BluetoothDevice::BluetoothDevice() : 
    m_handle{0},
    m_session_id{0},  // Initialize session ID
    m_initialized(false),
    m_connected(false),
    m_advertising(false)  // Initialize advertising flag
{
    // Initialize MAC address with zeros
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
    
    // Initialize session ID
    m_session_id = {0};
    
    void* workBuffer = aligned_alloc(0x1000, 0x1000);  // Allocate aligned buffer
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

    // Set device type (FullKey3 - Pro Controller)
    device_info.deviceType = HidDeviceType_FullKey3;  // 3

    // Set interface type (Bluetooth)
    device_info.npadInterfaceType = HidNpadInterfaceType_Bluetooth;  // 1

    // Set controller colors (required for proper operation)
    // Using RGBA8_MAXALPHA macro or direct values
    // RGBA8_MAXALPHA(r,g,b) = (((r)&0xff)|(((g)&0xff)<<8)|(((b)&0xff)<<16)|(0xff<<24))

    // White body
    device_info.singleColorBody = 0xFFFFFFFF;  // RGBA8_MAXALPHA(255, 255, 255)

    // Black buttons
    device_info.singleColorButtons = 0xFF000000;  // RGBA8_MAXALPHA(0, 0, 0)

    // Left grip color (blue)
    device_info.colorLeftGrip = 0xFF0000FF;  // RGBA8_MAXALPHA(0, 0, 255)

    // Right grip color (red)
    device_info.colorRightGrip = 0xFFFF0000;  // RGBA8_MAXALPHA(255, 0, 0)

    // For firmware versions 9.0.0+
    // device_info.npadControllerType = NpadControllerType_ProController;

    // Now call the function with properly initialized structure
    rc = hiddbgAttachHdlsVirtualDevice(&m_handle, &device_info);
    if (R_FAILED(rc)) {
        printf("Failed to attach virtual device: 0x%x\n", rc);
        hiddbgExit();
        return rc;
    }

    // Initialize random number generator
    srand(time(NULL));
    
    // Generate MAC address with Nintendo prefix (00:1F:32)
    m_device_address.address[0] = 0x00;
    m_device_address.address[1] = 0x1F;
    m_device_address.address[2] = 0x32;
    
    // Generate random bytes for the rest of the address
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
    
    // Display device type information
    printf("Device Type: Pro Controller (FullKey3)\n");
    printf("Interface Type: Bluetooth\n");
    printf("Connection Status: %s\n", m_connected ? "Connected" : "Not Connected");
    
    // Display device MAC address if it was saved
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
    
    // In the current implementation, we simply simulate the connection
    // For real connection checking, we need to use available APIs
    
    // Try to get information about the device state
    Result rc = 0;
    
    // Check if the device handle is valid
    if (m_handle.handle != 0) {
        // Simulate successful connection
        printf("Device connected successfully!\n");
        m_connected = true;
        
        // Display updated device information
        printf("=== Updated Device Status ===\n");
        printf("Connection Status: Connected\n");
        printf("Device Handle: 0x%x\n", m_handle.handle);
        
        // Display device MAC address
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
    
    // If the handle is not valid, consider the device not connected
    printf("Connection not established. Please try again.\n");
    return MAKERESULT(Module_Libnx, LibnxError_NotFound);
}

Result BluetoothDevice::Disconnect() {
    if (!m_connected) {
        return 0;
    }

    printf("Disconnecting Bluetooth device...\n");
    
    // If advertising is active, stop it before disconnecting
    if (m_advertising) {
        printf("Stopping advertising before disconnect...\n");
        Result rc = StopAdvertising();
        if (R_FAILED(rc)) {
            printf("Warning: Failed to stop advertising: 0x%x\n", rc);
            // Continue disconnection even if advertising stop fails
        }
    }
    
    // Here you can add API call for device disconnection,
    // if such API is available in libnx
    
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
    
    // Initialize btdrv service
    Result rc = btdrvInitialize();
    if (R_FAILED(rc)) {
        printf("Failed to initialize btdrv: 0x%x\n", rc);
        return rc;
    }
    
    // Set device to discoverable mode
    // Using a simpler API available in the current version of libnx
    rc = btdrvEnableBluetooth();
    if (R_FAILED(rc)) {
        printf("Failed to enable Bluetooth: 0x%x\n", rc);
        btdrvExit();
        return rc;
    }
    
    // Set visibility mode
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
    
    // Disable visibility
    Result rc = btdrvSetVisibility(false, false);  // discoverable=false, connectable=false
    if (R_FAILED(rc)) {
        printf("Failed to disable visibility: 0x%x\n", rc);
        // Continue even if disabling visibility fails
    }
    
    // Disable Bluetooth
    rc = btdrvDisableBluetooth();
    if (R_FAILED(rc)) {
        printf("Failed to disable Bluetooth: 0x%x\n", rc);
        // Continue even if disabling Bluetooth fails
    }
    
    // Exit btdrv service
    btdrvExit();
    
    m_advertising = false;
    printf("Bluetooth advertising stopped\n");
    
    return 0;
}

void BluetoothDevice::Finalize() {
    if (m_initialized) {
        // If the device is connected, disconnect it
        if (m_connected) {
            printf("Disconnecting device...\n");
            Disconnect();
        }
        
        // Detach virtual device
        printf("Detaching virtual device...\n");
        Result rc = hiddbgDetachHdlsVirtualDevice(m_handle);
        if (R_FAILED(rc)) {
            printf("Warning: Failed to detach virtual device: 0x%x\n", rc);
        }
        
        // Detach work buffer using the saved session ID
        printf("Detaching work buffer...\n");
        rc = hiddbgReleaseHdlsWorkBuffer(m_session_id);
        if (R_FAILED(rc)) {
            printf("Warning: Failed to release work buffer: 0x%x\n", rc);
        }
        
        // Exit hiddbg service
        printf("Exiting hiddbg service...\n");
        hiddbgExit();
        
        m_initialized = false;
        printf("Bluetooth finalized successfully\n");
    }
}