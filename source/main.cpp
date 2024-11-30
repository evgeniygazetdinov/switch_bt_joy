// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <switch.h>
// #include <string.h>
#include "bluetooth_handler.h"

// See also libnx pad.h / hid.h.

// Main program entrypoint

std::string random_string(){

			int randomNum = rand() % 101;

				std::string str = std::to_string(randomNum);   
				return str;
		}
const int KEY_X = 4;
const int KEY_Y = 28;
const int KEY_A = 5;
const int KEY_B = 27;
const int KEY_PLUS = 1024;
const int KEY_MINUS = 2048;

bool mainMenu()
{
	printf("\n\n-------- Main Menu --------\n");
	printf("Press B to run bluetooth init\n");


    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);




	while (appletMainLoop())
	{
			
		padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);
   
		if (kDown & KEY_X)
		{
			// return install();
            printf("X");
		}
		else if (kDown & KEY_Y)
		{
			// return install();
            printf("Y");
		}
		else if (kDown & KEY_A) // A
		{
			// return install();
            printf("A");
		}
		else if (kDown & KEY_B) // A
		{
			// return install();
            printf("B");
			BluetoothHandler btHandler;
			bool is_running = btHandler.initialize();
			if(!is_running){
				printf("is not running");

			}else{
				printf("is running");
			}
		
			
			// // Начинаем поиск устройств
			// printf("\x1b[1;1HStarting Bluetooth advertising...");
			// if (!btHandler.startAdvertising()) {
			// 	printf("\x1b[2;1HFailed to start advertising\n");
			// 	// consoleExit(NULL);
			// 	// return 1;
			// }
			
			// printf("\x1b[2;1HWaiting for Bluetooth connection...");
			// printf("\x1b[3;1HPress B to cancel");
		}
		else if(kDown & KEY_MINUS){
			break;
		}
		// elif
		// std::string stri;
		// std::string str = std::to_string(kDown);
		// printf(str.c_str()); 
		consoleUpdate(NULL);
	}
	return true;
}



int main(int argc, char* argv[])
{
    



    fsInitialize();

    consoleInit(NULL);
    mainMenu();
    consoleExit(NULL);
    return 0;
}