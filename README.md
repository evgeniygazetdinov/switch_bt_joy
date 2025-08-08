# Switch Bluetooth Joy

Extended Bluetooth functionality implementation for Nintendo Switch, allowing the console to be used in joystick mode

## Features

- Support for Host and Device modes
- Advanced Bluetooth connection management
- HID event handling
- Support for various Bluetooth device types
- Built-in error handling and reporting

## Requirements

- devkitPro with devkitA64 installed
- libnx (latest version)
- Nintendo Switch firmware with Bluetooth support

## Installation

1. Install devkitPro and required components:
```bash
# On Linux
wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.deb
sudo dpkg -i devkitpro-pacman.deb
sudo dkp-pacman -S switch-dev
```

2. Set up environment variables:
```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export DEVKITPPC=/opt/devkitpro/devkitPPC
```

3. Clone the repository:
```bash
git clone https://github.com/yourusername/switch_bt_joy.git
cd switch_bt_joy
```

4. Build the project:
```bash
make clean
make
```

## Usage

1. Copy the `switch_bt_joy.nro` file to your Nintendo Switch's SD card in the `/switch/` folder
2. Launch the application through the Homebrew menu

### Main Functions

- Bluetooth initialization:
```cpp
BluetoothHandler bt;
if (!bt.initialize()) {
    // Error handling
}
```

- Enable device discovery mode:
```cpp
if (!bt.startAdvertising()) {
    // Error handling
}
```

- Wait for connection:
```cpp
if (!bt.waitForConnection(5000)) { // 5 second timeout
    // Error handling
}
```

## Project Structure

```
switch_bt_joy/
├── source/
│   ├── bluetooth/
│   │   ├── bluetooth_core.cpp
│   │   ├── bluetooth_core.hpp
│   │   └── bluetooth_types.hpp
│   ├── bluetooth_handler.cpp
│   ├── bluetooth_handler.h
│   └── main.cpp
├── Makefile
├── icon.jpg
└── README.md
```

## Architecture

The project uses a three-tier architecture:

1. **Top level** (`bluetooth_handler.h/cpp`):
   - Provides a convenient API for working with Bluetooth
   - Manages connection state
   - Handles errors

2. **Middle level** (`bluetooth_core.hpp/cpp`):
   - Abstracts system calls
   - Implements basic Bluetooth operations
   - Manages operating modes

3. **Bottom level** (`bluetooth_types.hpp`):
   - Defines data types
   - Provides constants and enumerations
   - Ensures type compatibility

## Known Issues

- Possible compatibility issues with some firmware versions
- Limited support for certain types of Bluetooth devices
- Testing required on different system software versions

## Contributing

1. Fork the repository
2. Create a branch for your changes
3. Make your changes
4. Submit a pull request

## License

MIT License. See the LICENSE file for details.

## Authors

- Main developer: [Your name]
- Co-authors: [List of co-authors]

## Acknowledgements

- The devkitPro team for the excellent toolkit
- The Nintendo Switch homebrew community for support and testing
- All contributors for helping develop the project
