
#include <iostream>
#include "CPU.h"

void CPU::DisplayReg(uint8_t reg)
{
    std::cout << int(R[(reg & 0x03)]) << std::endl;
}

void CPU::DisplayRegAscii(uint8_t reg)
{
    std::cout << static_cast<char>(R[(reg & 0x03)]);
}

void CPU::DisplayRegP(uint8_t pairId)
{
    switch (pairId) {
        case 0: std::cout << ((R[0] << 8) | R[1]) << std::endl; break;
        case 1: std::cout << ((R[2] << 8) | R[3]) << std::endl; break;
    }
}

CPU::CPU(MEM* mem) : mem(mem)
{
    Reset();
}

void CPU::Reset()
{
    F = 0;
    R.fill(0);
    MDRH = MDRL = 0;
    MAR = 0;
    PC = 0x0000; 
    SP = 0xFE00; 
}

uint8_t CPU::Fetch8()
{
    return mem->Read(PC++); 
}

void CPU::Run()
{
    while (!halted)
    {
        Step();
    }
}

void CPU::SetPair(uint8_t pairId, uint16_t value) 
{
    switch (pairId) {
        case 0:
            R[0] = (value >> 8) & 0xFF;
            R[1] = value & 0xFF;
            break;
        case 1:
            R[2] = (value >> 8) & 0xFF;
            R[3] = value & 0xFF;
            break;
    }
}

uint16_t CPU::GetPair(uint8_t pairId) 
{
    switch (pairId) {
        case 0: return (R[0] << 8) | R[1]; 
        case 1: return (R[2] << 8) | R[3]; 
        default: return 0; 
    }
}

void CPU::SetFlag(uint8_t mask, bool set)
{
    if (set) F |= mask; 
    else F &= ~mask; 
}

bool CPU::GetFlag(uint8_t mask)
{
    return F & mask; 
}

void CPU::Step()
{
    uint8_t opcode = Fetch8();
    switch (opcode)
    {
        case 0x00: 
            break;
        case 0x01: 
            halted = true;
            break;
        case 0x02: 
        {
            MDRH = Fetch8() & 0x03; 
            MDRL = Fetch8(); 
            R[MDRH] = MDRL;
            break;
        }
        case 0x03:
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x3F) << 8) | MDRL;
            R[((MDRH & 0xC0) >> 6)] = mem->Read(0x8000 + MAR);
            break;
        }
        case 0x04: 
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x3F) << 8) | MDRL;
            mem->Write(0x8000 + MAR,R[((MDRH & 0xC0) >> 6)]);
            break;
        }
        case 0x05: 
        {
            MDRL = Fetch8();
            R[(MDRL & 0x0C) >> 2] = R[MDRL & 0x03];
            break;
        }
        case 0x06: 
        {
            MDRH = Fetch8();
            uint16_t result = R[(MDRH & 0x0C) >> 2] + R[(MDRH & 0x03)];
            SetFlag(V,((R[(MDRH & 0x0C) >> 2] ^ ~R[(MDRH & 0x03)]) & (R[(MDRH & 0x0C) >> 2] ^ result) & 0x80) != 0);
            SetFlag(N,result & 0x80);
            SetFlag(Z,(result & 0xFF) == 0);
            SetFlag(C,result > 0xFF);
            R[(MDRH & 0x30) >> 4] = static_cast<uint8_t>(result);
            break;
        }
        case 0x07: 
        {
            MDRH = Fetch8();
            uint16_t result = R[(MDRH & 0x0C) >> 2] + R[(MDRH & 0x03)] + GetFlag(C);
            SetFlag(V,((R[(MDRH & 0x0C) >> 2] ^ ~R[(MDRH & 0x03)]) & (R[(MDRH & 0x0C) >> 2] ^ result) & 0x80) != 0);
            SetFlag(N,result & 0x80);
            SetFlag(Z,(result & 0xFF) == 0);
            SetFlag(C,result > 0xFF);
            R[(MDRH & 0x30) >> 4] = static_cast<uint8_t>(result);
            break;
        }
        case 0x08: 
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] & R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x09:
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            PC = MAR;
            break;
        }
        case 0x0A: 
        {
            MDRL = Fetch8();
            DisplayReg(MDRL);
            break;
        }
        case 0x0B: 
        {
            MDRH = Fetch8();
            MDRL = Fetch8();
            uint8_t result = R[MDRH & 0x03] - MDRL;
            SetFlag(Z, result == 0);
            SetFlag(N, result & 0x80);
            break;
        }
        case 0x0C:
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            if (GetFlag(Z))
            { 
                PC = MAR;
            }
            break;
        }
        case 0x0D: 
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            if (GetFlag(N) != 1)
            { 
                PC = MAR;
            }
            break;
        }
        case 0x0E: 
        {
            MDRL = Fetch8();
            uint8_t Ra = MDRL & 0x03;
            uint8_t Rd = (MDRL & 0x0C) >> 2;
            uint16_t addr = R[Ra] + 0x8000; 
            R[Rd] = mem->Read(addr);
            break;
        }
        case 0x0F: 
        {
            MDRL = Fetch8();
            uint8_t Ra = MDRL & 0x03;
            uint8_t Rb = (MDRL & 0x0C) >> 2;
            uint16_t addr = R[Ra] + 0x8000;
            mem->Write(addr, R[Rb]);
            break;
        }
        case 0x10: 
        {
            MDRL = Fetch8();
            uint8_t Rd = (MDRL >> 4) & 0x0F;
            uint8_t pairId = MDRL & 0x01;
            uint16_t addr = GetPair(pairId) + 0x8000;
            R[Rd & 0x03] = mem->Read(addr); 
            break;
        }
        case 0x11: 
        {
            MDRL = Fetch8();
            uint8_t Rs = (MDRL >> 4) & 0x0F;
            uint8_t pairId = MDRL & 0x01;
            uint16_t addr = GetPair(pairId) + 0x8000;
            mem->Write(addr, R[Rs & 0x03]);
            break;
        }
        case 0x12: 
        {
            MDRL = Fetch8();                  
            uint8_t pairId = MDRL & 0x01;    
            uint8_t immHigh = Fetch8();      
            uint8_t immLow = Fetch8();       
            uint16_t imm = (immHigh << 8) | immLow;
            
            uint16_t regVal = GetPair(pairId);
            uint32_t result = regVal + imm;

            SetPair(pairId, static_cast<uint16_t>(result));

            SetFlag(Z, (result & 0xFFFF) == 0);
            SetFlag(N, result & 0x8000);
            SetFlag(C, result > 0xFFFF);
            break;
        }
        case 0x13: 
        {
            MDRH = Fetch8();                  
            MDRL = Fetch8();                  
            uint8_t Rd = MDRH & 0x03;
            
            uint16_t result = R[Rd] + MDRL;

            SetFlag(V, ((R[Rd] ^ ~MDRL) & (R[Rd] ^ result) & 0x80) != 0);
            SetFlag(N, result & 0x80);
            SetFlag(Z, (result & 0xFF) == 0);
            SetFlag(C, result > 0xFF);

            R[Rd] = static_cast<uint8_t>(result);
            break;
        }
        case 0x14: 
        {
            MDRL = Fetch8();
            DisplayRegP(MDRL & 0x01);
            break;
        }
        case 0x15: 
        {
            MDRL = Fetch8();
            DisplayRegAscii(MDRL);
            break;
        }
        case 0x16: 
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] | R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x17: 
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = ~(R[MDRL & 0x03] >> 2);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x18: 
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] ^ R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x19: 
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            if (GetFlag(N) == 1)
            { 
                PC = MAR;
            }
            break;
        }
        case 0x20: 
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;

            uint16_t temp = PC;
            mem->Write(SP--,(temp & 0xFF));
            mem->Write(SP--,((temp >> 8) & 0xFF));
            PC = MAR;
            break;
        }
        case 0x21:
        {
            uint8_t temp1, temp2;
            temp1 = mem->Read(++SP);
            temp2 = mem->Read(++SP);
            PC = (temp1 << 8) | temp2;
            break;
        }
        default:
            std::cerr << "Unknown opcode 0x" << std::hex << int(opcode) << "\n";
            halted = true;
            break;
    }
}