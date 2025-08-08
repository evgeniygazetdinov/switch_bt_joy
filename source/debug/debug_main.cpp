#include <stdio.h>
#include <unistd.h>
#include "mock_switch.hpp"

// Structure for storing button states
struct ButtonState {
    uint8_t buttons;
    int8_t stick_x;
    int8_t stick_y;
};

namespace {
    constexpr size_t HID_REPORT_SIZE = 3;
    
    // Same button masks as in main.cpp
    constexpr uint8_t BUTTON_A  = 0x01;
    constexpr uint8_t BUTTON_B  = 0x02;
    constexpr uint8_t BUTTON_X  = 0x04;
    constexpr uint8_t BUTTON_Y  = 0x08;
    constexpr uint8_t BUTTON_L  = 0x10;
    constexpr uint8_t BUTTON_R  = 0x20;
    constexpr uint8_t BUTTON_ZL = 0x40;
    constexpr uint8_t BUTTON_ZR = 0x80;
}

// Debug function to display button state
void PrintButtonState(const ButtonState& state) {
    printf("\rButtons: ");
    for(int i = 7; i >= 0; i--) {
        printf("%d", (state.buttons & (1 << i)) ? 1 : 0);
    }
    printf(" (");
    if(state.buttons & BUTTON_A) printf("A");
    if(state.buttons & BUTTON_B) printf("B");
    if(state.buttons & BUTTON_X) printf("X");
    if(state.buttons & BUTTON_Y) printf("Y");
    if(state.buttons & BUTTON_L) printf("L");
    if(state.buttons & BUTTON_R) printf("R");
    if(state.buttons & BUTTON_ZL) printf("ZL");
    if(state.buttons & BUTTON_ZR) printf("ZR");
    printf(") Stick: X=%d Y=%d    ", state.stick_x, state.stick_y);
    fflush(stdout);
}

int main() {
    // Initialize terminal for non-blocking input
    init_terminal();
    
    ButtonState state = {0};
    HidNpadButton kDown = 0;
    HidNpadButton kUp = 0;
    HidNpadButton kHeld = 0;
    bool button_states[8] = {false};  // Stores current state of each button
    
    printf("Debug Controller Emulator\n");
    printf("Controls:\n");
    printf("a/b/x/y - A/B/X/Y buttons\n");
    printf("l/r - L/R buttons\n");
    printf("z/c - ZL/ZR buttons\n");
    printf("arrows - D-pad\n");
    printf("q - quit\n\n");
    
    while(1) {
        // Clear press/release states
        kDown = 0;
        kUp = 0;
        
        // Read key
        char key = getch();
        
        // Exit on q
        if(key == 'q') break;
        
        // Skip if no key pressed
        if(key == 0) {
            usleep(1000); // Small delay to avoid CPU overload
            continue;
        }

        // Button index and corresponding mask
        int button_idx = -1;
        uint8_t button_mask = 0;
        
        // Determine which button was pressed
        switch(key) {
            case 'a': button_idx = 0; button_mask = BUTTON_A; kDown |= HidNpadButton_A; break;
            case 'b': button_idx = 1; button_mask = BUTTON_B; kDown |= HidNpadButton_B; break;
            case 'x': button_idx = 2; button_mask = BUTTON_X; kDown |= HidNpadButton_X; break;
            case 'y': button_idx = 3; button_mask = BUTTON_Y; kDown |= HidNpadButton_Y; break;
            case 'l': button_idx = 4; button_mask = BUTTON_L; kDown |= HidNpadButton_L; break;
            case 'r': button_idx = 5; button_mask = BUTTON_R; kDown |= HidNpadButton_R; break;
            case 'z': button_idx = 6; button_mask = BUTTON_ZL; kDown |= HidNpadButton_ZL; break;
            case 'c': button_idx = 7; button_mask = BUTTON_ZR; kDown |= HidNpadButton_ZR; break;
        }
        
        if(button_idx >= 0) {
            if(!button_states[button_idx]) {
                // Button was released - press it
                button_states[button_idx] = true;
                state.buttons |= button_mask;
            } else {
                // Button was pressed - release it
                button_states[button_idx] = false;
                state.buttons &= ~button_mask;
                kUp |= (1ULL << button_idx);  // Set release flag
            }
        }
        
        // Display current state
        PrintButtonState(state);
    }
    
    // Restore terminal settings
    restore_terminal();
    
    printf("\nDebug session ended\n");
    return 0;
}
