#include <fstream>
#include "Chip8.h"
#include <iostream>

using std::ifstream;
using std::cout;
using std::endl;

//the start address for instructions
const unsigned int START_ADDRESS = 0x200;

//fonts represent 16 characters ROMS need to write to the screen
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//seeds random generator with current time
Chip8::Chip8()
    : randGen(std::chrono::system_clock::now().time_since_epoch().count()) 
{
    pc = START_ADDRESS;

    //load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    //initialize RNG
    randByte = std::uniform_int_distribution<unsigned int>(0, 255U);
    // Main function pointer table
    //If opcode decode to 0x0 then call table0 function and go from there
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    // Subtables init to OP_NULL
    for (size_t i = 0; i <= 0xE; i++) {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    // TableF
    for (size_t i = 0; i <= 0x65; i++) {
        tableF[i] = &Chip8::OP_NULL;
    }
    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;

}

void Chip8::Table0() { 
    //keeps the lowest nibble and access pointer function stored in table0. defrence function so it calls whatever instruction(If lowst nibble is E it goes to table 0 and calls function OP_00EE)
    (this->*table0[opcode & 0x000Fu])(); 
}

void Chip8::Table8() { 
    (this->*table8[opcode & 0x000Fu])(); 
}

void Chip8::TableE() { 
    (this->*tableE[opcode & 0x000Fu])(); 
}

void Chip8::TableF() { 
    //this gets two nibbles because tableF needs to be disingished by last byte(How would you know to call Fx55 or Fx65 if they both end with 5)
    (this->*tableF[opcode & 0x00FFu])(); 
}

void Chip8::OP_NULL() {}


void Chip8::Cycle() {
    //Fetch next instruction in form of opcode. Uses bitwise OR to combine high byte and low byte. Shift memory[pc] 8 bits to the left to make it high bit
    //If memory[pc] = 0x6A and memory[pc + 1] = 0x0F then opcode = 0x6A0F which is 6xkk instruction(Vx = 0xA, kk=0x0F)
    opcode = (memory[pc] << 8u) | memory[pc + 1];

    //increment pc to get next instruction
    pc += 2;

    //Decode and Excute. >> 12u shifts it down 12 bits to get 0x6 which points to OP_6xkk
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    //decrement timers if not 0
    if(delayTimer > 0) {
        delayTimer--;
    }

    if(soundTimer > 0) {
        soundTimer--;
    }
}


void Chip8::LoadROM(char const* filename) {
    
    //open the file in binary mode and file pointer is positoned at end of file
    ifstream ROMfile(filename, std::ios::binary | std::ios::ate);

    if(!ROMfile.is_open()) {
        cout << "Error opening " << filename << endl;
        return;
    }

    //tellg or tell gets returns current position of file pointer. Since pointer at end it gives total file size
    std::streampos size = ROMfile.tellg();
    
    //allocate memory because heap has more space
    char* buffer = new char[size];

    //go back to start of file and fill buffer with file's contents
    ROMfile.seekg(0, std::ios::beg);
    ROMfile.read(buffer, size);
    ROMfile.close();

    //load ROM contents into memory start at 0x200
    for (long i = 0; i < size; ++i) {
        memory[START_ADDRESS + i] = buffer[i];
    }

    delete[] buffer;

}

void Chip8::OP_00E0() {
    //set video buffer to zero
    memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE() {
    //since top of stack has instruction past the one that called subroutine, just deincrement by 1
    sp--;
    pc = stack[sp];
}

void Chip8::OP_1nnn() {
    uint16_t address = opcode & 0x0FFFu;
    //set program counter to that address
    pc = address; 
}

void Chip8::OP_2nnn() {
    uint16_t address = opcode & 0xFFFu;

    //put current pc at top of stack so we are able to return(PC did +=2 in cycle so it holds next instruction)
    stack[sp] = pc;
    sp++;
    pc = address;
}

void Chip8::OP_3xkk() {
    //bitwise masking. We AND the mask(0x0F00) and zero all the other parts. >>8u shifts the value right 8 bits. This simply leaves whatever the opcode value is at the end or decimal(0-15)
    //This makes it so Vx = V10 or register 10
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    //collect the last 8 bits or byte at end of op code(the actual value not register number)
    uint8_t byte = opcode & 0x00FFu;

    //pc has already been incremented in cycle stage. So just increment again to skip instruction
    if(registers[Vx] == byte) {
        pc += 2;
    }

}

void Chip8::OP_4xkk() {
    
    //same reasoning as 3xkk. Bitmask and shift the unchanged value to the right to get whatever opcode register index is stored
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t byte = opcode & 0x00FFu;

    if(registers[Vx] != byte) {
        pc += 2;
    }
}

void Chip8::OP_5xy0() {
    
    //chip 8 opcode have the Vx register at bits 8-11, so we keep those bits
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    //Vy register is located at bits 4-7
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    //compare two registers
    if(registers[Vx] == registers[Vy]) {
        pc += 2;
    }
}

void Chip8::OP_6xkk() {
    
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

void Chip8::OP_7xkk() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = registers[Vx] + byte;
}

void Chip8::OP_8xy0() {
    
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1() {
    
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    //performs bitwse OR on Vx and Vy values
    registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    //performs bitwise AND on Vx and Vy values
    registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3() {
    
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    //performs bitwise XOR on Vx and Vy values
    registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    //make it 16 bits in case sum goes over 8 bits
    uint16_t sum = registers[Vx] + registers[Vy];

    //Set Vf to 1 if larger than 8 bits or 255
    if(sum > 255U) {
        registers[0xF] = 1;
    }
    else {
        registers[0xF] = 0;
    }

    //keeps the last 8 bits using AND
    registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] > registers[Vy]) {
        registers[0xF] = 1;
    }
    else {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vx] - registers[Vy];

}

void Chip8::OP_8xy6() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    //save least signficant bit(rightmost bit) in VF
    registers[0xF] = (registers[Vx] & 0x1u);

    //does a bitwise rightsift by 1 or just simplying dividing by 2
    registers[Vx] >>= 1;
}

void Chip8::OP_8xy7() {
    
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vy] > registers[Vx]) {
        registers[0xF] = 1;
    }
    else {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    //get most signficant bit(leftmost bit)
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    //leftmost shift by 1 or multiplication by 2
    registers[Vx] <<= 1;
}

void Chip8::OP_9xy0() {

    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] != registers[Vy]) {
        pc += 2;
    }
}

void Chip8::OP_Annn() {
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

void Chip8::OP_Bnnn() {
    uint16_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;
}

void Chip8::OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    //get random byte
    registers[Vx] = static_cast<uint8_t>(randByte(randGen)) & byte;
}

void Chip8::OP_Dxyn() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    //get last 4 bits for height or number of rows(1 byte each)
    uint8_t height = opcode & 0x000Fu;

    //wrap x and y within position video width and video height position, so we don't go over bounds
    uint8_t xPos = registers[Vx] % 64;
	uint8_t yPos = registers[Vy] % 32;

    //default collision
    registers[0xF] = 0;

    for(unsigned int row = 0; row < height; row++) {
        //get our 1 byte or row
        uint8_t spriteByte = memory[index + row];

        //do 8 iterations because a single byte is 8 bits
        for(unsigned int col = 0; col < 8; col++) {
            //gets the leftmost bit and shift right by much col is
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            //to access pixel(x, y) you calculate it with y * width + x in our video array. Use 32 bits for color values
            //uint32_t* screenPixel = &video[(yPos + row) * 64 + (xPos + col)];

            uint32_t screenIndex = (yPos + row) * 64 + (xPos + col);
            //check if sprite pixel is ON or 1
            if(spritePixel) {

                //check if screenPixel is white or ON for collision checking
                if(video[screenIndex] == 1) {
                    registers[0xF] = 1;
                }

                //flips pixel
                video[screenIndex] ^= 1;
            }
        }
    }
}

void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    //returns true as long as key is not 0(not pressed)
    if(keypad[key]) {
        //increment pc to skip instruction
        pc += 2;
    }
}

void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t key = registers[Vx];

    //check if keypad[key] = 0
    if(!keypad[key]) {
        pc += 2;
    }
}

void Chip8::OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    bool detected = false;
    for(int i = 0; i < 16; i++) {
        if(keypad[i]) {
            registers[Vx] = i;
            detected = true;
            break;
        }
    }

    if(!detected) {
        //if key is not detected decrement by 2 to wait. This simply just runs the same instruction repeatedly
        pc -= 2;
    }
}

void Chip8::OP_Fx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}

void Chip8::OP_Fx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

void Chip8::OP_Fx29() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    //since font characters are 5 bytes each we can get the address of first byte of any character by taking offset from start address
    index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    //get the ones place. Divide by 10 to get next placement
    memory[index + 2] = value % 10;
    value /= 10;

    //tens place
    memory[index + 1] = value % 10;
    value /=10;

    //hundreds place
    memory[index] = value % 10;
}

void Chip8::OP_Fx55() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; i++) {
        memory[index + i] = registers[i];
    }
}

void Chip8::OP_Fx65() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; i++) {
        registers[i] = memory[index + i];
    }
}