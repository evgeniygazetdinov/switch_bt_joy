// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include "bluetooth_handler.h"

// See also libnx pad.h / hid.h.

// Main program entrypoint
int main(int argc, char* argv[])
{
    consoleInit(NULL);
    
    // // Инициализация логгера
    // if (!ConnectionLogger::initialize("sdmc:/switch/bluetooth_connections.log")) {
    //     printf("Warning: Failed to initialize logger: %s\n", ConnectionLogger::getLastError());
    // }
    
    // Инициализация Bluetooth
    BluetoothHandler btHandler;
    if (!btHandler.initialize()) {
        printf("Failed to initialize Bluetooth\n");
        consoleExit(NULL);
        return 1;
    }
    
    // Начинаем поиск устройств
    printf("\x1b[1;1HStarting Bluetooth advertising...");
    if (!btHandler.startAdvertising()) {
        printf("\x1b[2;1HFailed to start advertising\n");
        consoleExit(NULL);
        return 1;
    }
    
    printf("\x1b[2;1HWaiting for Bluetooth connection...");
    printf("\x1b[3;1HPress B to cancel");
    
    // Конфигурация контроллера
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    
    // Ожидаем подключения или нажатия кнопки B для отмены
    while (!btHandler.isConnected() && appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        
        if (kDown & HidNpadButton_B) {
            printf("\x1b[4;1HCancelled by user\n");
            btHandler.stopAdvertising();
            consoleExit(NULL);
            return 0;
        }
        
        // Проверяем подключение каждые 100мс
        if (btHandler.waitForConnection(100)) {
            BluetoothDeviceInfo deviceInfo;
            if (btHandler.getConnectedDeviceInfo(&deviceInfo)) {
                // Выводим информацию на экран
                printf("\x1b[4;1HDevice connected!\n");
                printf("\x1b[5;1HName: %s\n", deviceInfo.name);
                printf("\x1b[6;1HAddress: %02X:%02X:%02X:%02X:%02X:%02X\n",
                       deviceInfo.address.address[0], deviceInfo.address.address[1],
                       deviceInfo.address.address[2], deviceInfo.address.address[3],
                       deviceInfo.address.address[4], deviceInfo.address.address[5]);
                printf("\x1b[7;1HRSSI: %d dBm\n", deviceInfo.rssi);
                
                // Логируем подключение
                // if (!ConnectionLogger::logConnection(deviceInfo)) {
                //     printf("\x1b[8;1HWarning: Failed to log connection: %s\n", 
                //            ConnectionLogger::getLastError());
                // }
            } else {
                printf("\x1b[4;1HDevice connected, but failed to get info: %s\n", 
                       btHandler.getLastErrorMessage());
            }
            break;
        }
        
        consoleUpdate(NULL);
    }
    
    if (!btHandler.isConnected()) {
        printf("\x1b[4;1HFailed to establish connection: %s\n", 
               btHandler.getLastErrorMessage());
        consoleExit(NULL);
        return 1;
    }
    
    // Ждем 3 секунды, чтобы пользователь мог прочитать информацию
    svcSleepThread(3000000000ULL);
    
    // Очищаем консоль и показываем основной интерфейс
    consoleClear();
    printf("\x1b[1;1HPress PLUS to exit.");
    printf("\x1b[2;1HBluetooth connected and ready!");
    printf("\x1b[3;1HPress buttons to send via Bluetooth");
    
    //Matrix containing the name of each key. Useful for printing when a key is pressed
    char keysNames[28][32] = {
        "A", "B", "X", "Y",
        "StickL", "StickR", "L", "R",
        "ZL", "ZR", "Plus", "Minus",
        "Left", "Up", "Right", "Down",
        "StickLLeft", "StickLUp", "StickLRight", "StickLDown",
        "StickRLeft", "StickRUp", "StickRRight", "StickRDown",
        "LeftSL", "LeftSR", "RightSL", "RightSR",
    };

    u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0; //In these variables there will be information about keys detected in the previous frame

    printf("\x1b[4;1HLeft joystick position:");
    printf("\x1b[6;1HRight joystick position:");

    // Main loop
    while(appletMainLoop())
    {
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        // padGetButtons returns the set of buttons that are currently pressed
        u64 kHeld = padGetButtons(&pad);

        // padGetButtonsUp returns the set of buttons that have been
        // newly released in this frame compared to the previous one
        u64 kUp = padGetButtonsUp(&pad);

        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        // if (!btHandler.isConnected()) {
        //     BluetoothDeviceInfo deviceInfo;
        //     if (btHandler.getConnectedDeviceInfo(&deviceInfo)) {
        //         ConnectionLogger::logDisconnection(deviceInfo);
        //     }
        //     printf("\x1b[4;1HBluetooth connection lost!\n");
        //     break;
        // }

        // Do the keys printing only if keys have changed
        if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld)
        {
            // Clear console
            consoleClear();

            // These two lines must be rewritten because we cleared the whole console
            printf("\x1b[1;1HPress PLUS to exit.");
            printf("\x1b[2;1HBluetooth connected and ready!");
            printf("\x1b[3;1HPress buttons to send via Bluetooth");
            printf("\x1b[4;1HLeft joystick position:");
            printf("\x1b[6;1HRight joystick position:");
            printf("\x1b[7;1H"); //Move the cursor to the seventh row because on the previous ones we'll write the joysticks' position

            // Check if some of the keys are down, held or up
            int i;
            for (i = 0; i < 28; i++)
            {
                if (kDown & BIT(i)) {
                    btHandler.sendKeyPress(i);
                    printf("\x1b[%d;1H%s sent via BT\n", 8 + i, keysNames[i]);
                }
                if (kHeld & BIT(i)) printf("\x1b[%d;1H%s held\n", 8 + i, keysNames[i]);
                if (kUp & BIT(i)) printf("\x1b[%d;1H%s up\n", 8 + i, keysNames[i]);
            }
        }

        // Set keys old values for the next frame
        kDownOld = kDown;
        kHeldOld = kHeld;
        kUpOld = kUp;

        // Read the sticks' position
        HidAnalogStickState analog_stick_l = padGetStickPos(&pad, 0);
        HidAnalogStickState analog_stick_r = padGetStickPos(&pad, 1);

        // Print the sticks' position
        printf("\x1b[5;1H%04d; %04d", analog_stick_l.x, analog_stick_l.y);
        printf("\x1b[7;1H%04d; %04d", analog_stick_r.x, analog_stick_r.y);

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }

    btHandler.disconnect();
    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    return 0;
}