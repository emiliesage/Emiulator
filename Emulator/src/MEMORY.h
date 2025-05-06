#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <vector>
#include <cstdint>

class MEM 
{
    public:

        uint8_t Read(uint16_t addr) const; 
        void Write(uint16_t addr, uint8_t val); 
        void LoadProgram(const std::vector<uint8_t>& program, uint16_t start); 
        

    private:
        
        std::array<uint8_t, 0x10000> data{}; 

};

#endif