#pragma once
#include <cstdint>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>

// Mock definitions from libnx
using HidNpadButton = uint64_t;

// Emulation of Switch button constants
constexpr HidNpadButton HidNpadButton_A     = 1ULL << 0;
constexpr HidNpadButton HidNpadButton_B     = 1ULL << 1;
constexpr HidNpadButton HidNpadButton_X     = 1ULL << 2;
constexpr HidNpadButton HidNpadButton_Y     = 1ULL << 3;
constexpr HidNpadButton HidNpadButton_L     = 1ULL << 4;
constexpr HidNpadButton HidNpadButton_R     = 1ULL << 5;
constexpr HidNpadButton HidNpadButton_ZL    = 1ULL << 6;
constexpr HidNpadButton HidNpadButton_ZR    = 1ULL << 7;
constexpr HidNpadButton HidNpadButton_Up    = 1ULL << 8;
constexpr HidNpadButton HidNpadButton_Down  = 1ULL << 9;
constexpr HidNpadButton HidNpadButton_Left  = 1ULL << 10;
constexpr HidNpadButton HidNpadButton_Right = 1ULL << 11;

// Function for non-blocking key reading
inline int kbhit(void) {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

inline void init_terminal(void) {
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(0, TCSANOW, &term);
}

inline void restore_terminal(void) {
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(0, TCSANOW, &term);
}

inline char getch(void) {
    char buf = 0;
    if (kbhit()) {
        if (read(STDIN_FILENO, &buf, 1) < 0) {
            return 0;
        }
    }
    return buf;
}
