#include "MEMORY.h"

uint8_t MEM::Read(uint16_t addr) const
{
    return data[addr]; 
}

void MEM::Write(uint16_t addr, uint8_t val)
{
    data[addr] = val; 
}

void MEM::LoadProgram(const std::vector<uint8_t>& program, uint16_t start)
{
    for (size_t i = 0; i < program.size(); ++i) 
    {
        data[start + i] = program[i]; 
    }
}

