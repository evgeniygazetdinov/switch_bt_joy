#include <stdio.h>
#include <switch.h>
#include "bluetooth/bluetooth_device.hpp"

// Структура для хранения состояния кнопок
struct ButtonState {
    uint8_t buttons;  // Один байт для всех кнопок, каждый бит = одна кнопка
    int8_t stick_x;   // Положение стика по X (-127 до 127)
    int8_t stick_y;   // Положение стика по Y (-127 до 127)
};

// namespace {
//     // Размер HID репорта: 1 байт кнопки + 2 байта стик
//     constexpr size_t HID_REPORT_SIZE = 3;
    
//     // Битовые маски для каждой кнопки в байте HID репорта
//     constexpr uint8_t BUTTON_A  = 0x01;  // Бит 0
//     constexpr uint8_t BUTTON_B  = 0x02;  // Бит 1
//     constexpr uint8_t BUTTON_X  = 0x04;  // Бит 2
//     constexpr uint8_t BUTTON_Y  = 0x08;  // Бит 3
//     constexpr uint8_t BUTTON_L  = 0x10;  // Бит 4
//     constexpr uint8_t BUTTON_R  = 0x20;  // Бит 5
//     constexpr uint8_t BUTTON_ZL = 0x40;  // Бит 6
//     constexpr uint8_t BUTTON_ZR = 0x80;  // Бит 7
// }

// // Преобразование состояния кнопок в HID репорт
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
    printf("\n\n-------- Main Menu --------\n");
    printf("Press B to initialize Bluetooth\n");
    printf("Press - to exit\n");

    // Создаем Bluetooth устройство
    BluetoothDevice device;
    ButtonState button_state = {};
    //uint8_t hid_report[HID_REPORT_SIZE] = {};

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);


    while (appletMainLoop()) {
        // Сканируем ввод
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        bool checking_connections = false;
        if (kDown & KEY_MINUS) {
            printf("Exiting...\n");
            return false;
        }

        if (kDown & KEY_B) {
            printf("Initializing Bluetooth...\n");
            Result result_of_initialize = device.Initialize();
            if (R_FAILED(result_of_initialize)) {
                printf("Failed to initialize Bluetooth: %x\n", result_of_initialize);
                continue;
            }
            else{
                printf("Starting connection check...\n");
                checking_connections = true;
            }
        }

        // Проверяем подключения если включено
        if (checking_connections) {
            Result result_of_wait = device.WaitForConnection();
            if (R_FAILED(result_of_wait)) {
                printf("Connection check failed: %x\n", result_of_wait);
                checking_connections = false;
            }
        }
        consoleUpdate(NULL);
        svcSleepThread(100000000ULL); // Спим 100мс чтобы не грузить процессор
    }

    return true;
}

int main(int argc, char* argv[]) {
    // Инициализация консоли

	consoleInit(NULL);
    mainLoop();
    consoleExit(NULL);
    return 0;
}