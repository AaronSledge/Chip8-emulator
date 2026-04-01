#pragma once
#include <cstdint>
#include <random>
#include <chrono>

class Chip8 {
    public:
        //{} initializes value(s) to zero

        //create an array of size 16. uint8 reprsents 8 bits
        uint8_t registers[16]{};
        //create an array of 4k+ bytes
        uint8_t memory[4096]{};
        //index register. Stores memory addresses for operations
        uint16_t index{};
        //program counter: Hold the address of next instruction to execute
        uint16_t pc{};
        //keeps track of order of execution for CALL instruction
        uint16_t stack[16]{};
        //tells us where most recent value is
        uint8_t sp{};
        uint8_t delayTimer{};
        //difference between this timer is that a tune will buzz if non zero
        uint8_t soundTimer{};
        //input keys that for first 16 hex values
        uint8_t keypad[16]{};
        //stores graphics for display
        uint32_t video[64 * 32]{};
        //spefic instruction
        uint16_t opcode;
        //rand num generator. Need this because there is an instruction that places a random number in a register
        std::default_random_engine randGen;
	    std::uniform_int_distribution<unsigned int> randByte;
        Chip8();
        //before excuting instructions we need to those instructions in memory
        void LoadROM(char const* filename);
        //Instruction CLS: Clear display
        void OP_00E0();
        //Instruction RET: Return from a subroutine
        void OP_00EE();
        //Instruction JP addr: Jump to location nnn
        void OP_1nnn();
        //Instruction CALL addr: Call subroutine at nnn
        void OP_2nnn();
        //Instruction SE Vx, byte: Skip next instruction if Vx = kk
        void OP_3xkk();
        //Insturction SNE Vx, byte: Skip next instruction if vx != kk
        void OP_4xkk();
        //Instruction SE Vx, Vy: Skip next instruction if vx = vy
        void OP_5xy0();
        //Instruction LD Vx, byte: Set Vx to kk
        void OP_6xkk();
        //Instruction ADD Vx, byte: Set Vx = Vx + kk
        void OP_7xkk();
        //Instruction LD Vx, Vy: Set Vx = Vy
        void OP_8xy0();
        //Instruction OR Vx, Vy: Set Vx = Vx OR Vy
        void OP_8xy1();
        //Instruction AND Vx, Vy: Set Vx = Vx AND Vy
        void OP_8xy2();
        //Instruction XOR Vx, Vy: Set Vx = Vx XOR Vy
        void OP_8xy3();
        //Instruction ADD Vx, Vy: Set Vx = Vx + Vy and set VF = carry
        void OP_8xy4();
        //Instruction SUB Vx, Vy: Set Vx = Vx - Vy, set VF = NOT borrow
        void OP_8xy5();
        //Instruction SHR Vx: Set Vx = Vx SHR 1
        void OP_8xy6();
        //Instruction SUBN Vx, Vy: Set Vx = Vy - Vx, set VF = NOT borrow
        void OP_8xy7();
        //Instruction SHL Vx {, Vy}: Set Vx = Vx SHL 1
        void OP_8xyE();
        //Instruction SNE Vx, Vy: Skip next instruction if Vx != Vy
        void OP_9xy0();
        //Instruction LD I, addr: Set I = nnn
        void OP_Annn();
        //Instruction JP V0, addr: Jump to location nnn + V0
        void OP_Bnnn();
        //Instruction RND Vx, byte: Set vx = random byte AND kk
        void OP_Cxkk();
        //Instruction DRW Vx, Vy, nibble: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        void OP_Dxyn();
        //Instruction SKP Vx: Skip next intrustion if key with the value of Vx is pressed
        void OP_Ex9E();
        //Instruction SKNP Vx: skip next instruction if key with the value of Vx is NOT pressed
        void OP_ExA1();
        //Instruction LD Vx, DT: Set Vx = delay timer value
        void OP_Fx07();
        //Instruction LD Vx, k: Wait for a key press, store the value of key in Vx
        void OP_Fx0A();
        //Instruction LD DT, Vx: Set delay timer = Vx
        void OP_Fx15();
        //Instruction LD ST, Vx: Set sound timer = Vx
        void OP_Fx18();
        //Instruction ADD I, Vx: Set I = I + Vx
        void OP_Fx1E();
        //Instruction LD F, Vx: Set I = location of sprite for digit Vx
        void OP_Fx29();
        //Instruction LD B, Vx: Store BCD reprsentation of Vx in memory locations I, I+1, I+2
        void OP_Fx33();
        //Instruction LD [I], Vx: Store registers V0 through Vx in memory starting at Location I
        void OP_Fx55();
        //Instruction LD Vx, [I]: Read registers V0 through Vx from memory starting at Location I
        void OP_Fx65();


        //Secondary table to hold first digit $0
        void Table0();
        //Secondary table to hold first digit $8
        void Table8();
        //Secondary table to hold first digit $E
        void TableE();
        //Secondary table to hold first digit $F
        void TableF();
        //dummy null used as fefault function if function pointer is not set
        void OP_NULL();
        //Function pointer typedef and tables
        typedef void (Chip8::*Chip8Func)();
        Chip8Func table[0xF + 1];
        Chip8Func table0[0xE + 1];
        Chip8Func table8[0xE + 1];
        Chip8Func tableE[0xE + 1];
        Chip8Func tableF[0x65 + 1];

        //Fetch decode, excute cycle
        void Cycle();

};