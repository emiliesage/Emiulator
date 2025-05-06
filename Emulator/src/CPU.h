#ifndef CPU_H_
#define CPU_H_

#include <array>
#include <vector>
#include <cstdint>
#include "MEMORY.h"

class CPU 
{
    public:
        void Step(); 
        CPU(MEM* mem); 
        void Reset(); 
        void Run(); 
        void DisplayReg(uint8_t reg);
        void DisplayRegP(uint8_t pairId);
        void DisplayRegAscii(uint8_t reg);

    private:

        uint8_t F; 
        std::array<uint8_t, 4> R{}; 
        uint8_t MDRH, MDRL; 
        uint16_t MAR; 
        uint16_t PC, SP; 

        bool halted = false; 
        MEM* mem; 

        enum FLAGS
        {
            C = 0x01, Z = 0X02 , I = 0x04, 
            N = 0x8, V = 0x10 
        };
        
        uint8_t Fetch8(); 
        void SetFlag(uint8_t mask, bool set); 
        bool GetFlag(uint8_t mask); 
        uint16_t GetPair(uint8_t pairId); 
        void SetPair(uint8_t pairId, uint16_t value);
};

#endif