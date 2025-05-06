#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <vector>
#include <cstdint>

//Class for memory
class MEM 
{
    public:

        uint8_t Read(uint16_t addr) const; //Read from memory
        void Write(uint16_t addr, uint8_t val); //Write to memory
        void LoadProgram(const std::vector<uint8_t>& program, uint16_t start); //Load program from array
        

    private:
        
        std::array<uint8_t, 0x10000> data{}; //Memory data

};

#endif