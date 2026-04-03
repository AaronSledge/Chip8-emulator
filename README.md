# CHIP8 EMULATOR
A chip8 emulator is used to play older 8 bit games. This was coded entirely in c++

# REQUIRMENTS
Cmake installed
SDL3 installed
C++ compiler

# HOW TO PLAY:
After copying the repo, type in the command line: 
cmake -S . -B build
cmake --build build
Add any .ch8 files into the debug folder inside the build file. 
Then CD into the build folder, then debug folder. Run this line of code in the command line.
.\Chip8Emu 10 5 test_opcode.ch8

Have fun!

# BIGGEST TAKEAWAYS
The biggest takeaway from this project is it taught me lower level programming. The concepts of PC, bit masking, OPCODES, etc are used in this project. I've written comments explaining each function and how it works in case you want to follow along.
