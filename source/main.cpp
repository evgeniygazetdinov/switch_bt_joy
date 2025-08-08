#include <stdio.h>
#include <switch.h>
#include "bluetooth/bluetooth_device.hpp"
#include <ctime>
#include <cstdlib>

// Structure for storing button states
struct ButtonState {
    uint8_t buttons;  // One byte for all buttons, each bit = one button
    int8_t stick_x;   // Stick position on X axis (-127 to 127)
    int8_t stick_y;   // Stick position on Y axis (-127 to 127)
};

// namespace {
//     // HID report size: 1 byte buttons + 2 bytes stick
//     constexpr size_t HID_REPORT_SIZE = 3;
    
//     // Bit masks for each button in the HID report byte
//     constexpr uint8_t BUTTON_A  = 0x01;  // Bit 0
//     constexpr uint8_t BUTTON_B  = 0x02;  // Bit 1
//     constexpr uint8_t BUTTON_X  = 0x04;  // Bit 2
//     constexpr uint8_t BUTTON_Y  = 0x08;  // Bit 3
//     constexpr uint8_t BUTTON_L  = 0x10;  // Bit 4
//     constexpr uint8_t BUTTON_R  = 0x20;  // Bit 5
//     constexpr uint8_t BUTTON_ZL = 0x40;  // Bit 6
//     constexpr uint8_t BUTTON_ZR = 0x80;  // Bit 7
// }

// // Convert button state to HID report
// void CreateHidReport(const ButtonState& state, uint8_t* report) {
//     report[0] = state.buttons;
//     report[1] = state.stick_x;
//     report[2] = state.stick_y;
// }

const int KEY_X = 4;
const int KEY_Y = 28;
const int KEY_A = 5;
const int KEY_B = 27;
const int KEY_PLUS = 1024;
const int KEY_MINUS = 2048;

bool mainLoop() {
    printf("\n\n------------------------------ Main Menu ------------------------------\n");
    printf("Press B to initialize Bluetooth\n");
    printf("Press - to exit\n");
    printf("\n\n-----------------------------------------------------------------------\n");

    // Create Bluetooth device
    BluetoothDevice device;
    ButtonState button_state = {};
    //uint8_t hid_report[HID_REPORT_SIZE] = {};

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    bool should_exit = false;
    bool checking_connections = false;

    while (appletMainLoop() && !should_exit) {
        // Scan input
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        
        if (kDown & KEY_MINUS) {
            printf("Exiting...\n");
            should_exit = true;
            
            // If the device was initialized, properly terminate it
            if (device.IsConnected()) {
                printf("Disconnecting Bluetooth device...\n");
                device.Disconnect();
            }
            
            // Continue executing the loop to properly terminate all processes
            continue;
        }

        if (kDown & KEY_B) {
            printf("Initializing Bluetooth...\n");
            Result result = device.Initialize();
            if (R_SUCCEEDED(result)) {
                device.PrintDeviceInfo();
                
                // Start Bluetooth advertising
                printf("Starting Bluetooth advertising...\n");
                result = device.StartAdvertising();
                if (R_FAILED(result)) {
                    printf("Failed to start advertising: 0x%x\n", result);
                }
                
                // Start checking connections
                checking_connections = true;
            } else {
                printf("Failed to initialize Bluetooth: 0x%x\n", result);
            }
        }

        // Check connections if enabled
        if (checking_connections) {
                Result result_of_wait = device.WaitForConnection();
                if (R_FAILED(result_of_wait)) {
                    printf("Connection check failed: %x\n", result_of_wait);
                    checking_connections = false;
                }
                svcSleepThread(100000000ULL); 
        }
        
        consoleUpdate(NULL);
        svcSleepThread(100000000ULL); // Sleep 100ms to avoid CPU overload
    }
    //device.Shutdown();

    // Properly free resources before exit
    printf("Cleaning up resources...\n");
    consoleUpdate(NULL);
    
    return true;
}

int main(int argc, char* argv[]) {
    // Initialize console
    consoleInit(NULL);
    
    // Initialize random number generator
    srand(time(NULL));
    
    mainLoop();
    
    // Properly close the console
    consoleExit(NULL);
    return 0;
}