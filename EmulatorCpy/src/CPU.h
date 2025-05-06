#ifndef CPU_H_
#define CPU_H_

#include <array>
#include <vector>
#include <cstdint>
#include "MEMORY.h"

// Class for the CPU
//Program memory starts at 0x0000
//Data Memory starts at 0x8000
//Stack pointer starts at 0xFE00
class CPU 
{
    public:
        void Step(); //Step one cycle through program
        CPU(MEM* mem); //Constructor takes in memory
        void Reset(); //Reset CPU
        void Run(); //Run CPU 
        void DisplayReg(uint8_t reg);
        void DisplayRegP(uint8_t pairId);
        void DisplayRegAscii(uint8_t reg);

    private:

        uint8_t F; //Accumulator and flag regesters
        std::array<uint8_t, 4> R{}; //Four General Purpose Registers
        uint8_t MDRH, MDRL; //Memory data high and low
        uint16_t MAR; //Memory address
        uint16_t PC, SP; //Program Counter and stack pointer

        bool halted = false; //Is halted?
        MEM* mem; //Memory pointer

        //Flags contain, Zero, Carry, Interrupt disable, Negative, and Overflow
        enum FLAGS
        {
            C = 0x01, Z = 0X02 , I = 0x04, 
            N = 0x8, V = 0x10 
        };
        
        uint8_t Fetch8(); //Fetch from memory
        void SetFlag(uint8_t mask, bool set); //set flag
        bool GetFlag(uint8_t mask); //get flag
        uint16_t GetPair(uint8_t pairId); 
        void SetPair(uint8_t pairId, uint16_t value);
};

#endif