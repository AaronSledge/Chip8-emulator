#include <chrono>
#include <iostream>
#include <string>
#include "Platform.h"

#include "Chip8.h"

//IMPORTANT: CD into build and debug and run this command to run code .\Chip8Emu 10 5 test_opcode.ch8
int main(int argc, char** argv) {

    
    //Make sure we got the executable file, video scale, delay, and ROM name arguments
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << "<Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }


    //converts string into intger so you can do calc. Like if scale is 10, you can do 64 * 10
    int videoScale = std::stoi(argv[1]);
    //converts string into integer, controls how much of a delay the cycle is
	int cycleDelay = std::stoi(argv[2]);
	char const* romFilename = argv[3];

    Platform platform("CHIP-8 Emulator", 64 * videoScale, 32 * videoScale, 64, 32);
    
    Chip8 chip8;

    chip8.LoadROM(romFilename);

    //pitch is number of bytes in one row of pixels. Calculate how many bypes each row needs
    int videoPitch = sizeof(chip8.video[0]) * 64;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;
    uint32_t displayBuffer[64 * 32];
    while(!quit) {
        quit = platform.ProcessInput(chip8.keypad);
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if(dt > cycleDelay) {
            lastCycleTime = currentTime;
            chip8.Cycle();
            for (int i = 0; i < 64 * 32; ++i) {
                displayBuffer[i] = chip8.video[i] ? 0xFFFFFFFF : 0xFF000000;
            }
            platform.Update(displayBuffer, videoPitch, 64 * videoScale, 32 * videoScale);
        }
    }

    return 0;
}