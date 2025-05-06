
#include <iostream>
#include "CPU.h"

/*
    Program memory starts at 0x0000
    Data memory starts at 0x8000
    Stack starts at 0xFE00
*/

//Display contents of a Register
void CPU::DisplayReg(uint8_t reg)
{
    std::cout << int(R[(reg & 0x03)]) << std::endl;
}

//Display contents of a register as a char
void CPU::DisplayRegAscii(uint8_t reg)
{
    std::cout << static_cast<char>(R[(reg & 0x03)]);
}

//Display contents of a register pair
void CPU::DisplayRegP(uint8_t pairId)
{
    switch (pairId) {
        case 0: std::cout << ((R[0] << 8) | R[1]) << std::endl; break;
        case 1: std::cout << ((R[2] << 8) | R[3]) << std::endl; break;
    }
}

//Constructor resets cpu
CPU::CPU(MEM* mem) : mem(mem)
{
    Reset();
}

//Reset clears flags, and empties registers
void CPU::Reset()
{
    F = 0;
    R.fill(0);
    MDRH = MDRL = 0;
    MAR = 0;
    PC = 0x0000; 
    SP = 0xFE00; 
}

//Read byte from memory
uint8_t CPU::Fetch8()
{
    return mem->Read(PC++); 
}

//Step through program while not haulted
void CPU::Run()
{
    while (!halted)
    {
        Step();
    }
}

//Set register pairs to 16 bit value
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

//Get 16 bit value from register pair
uint16_t CPU::GetPair(uint8_t pairId) 
{
    switch (pairId) {
        case 0: return (R[0] << 8) | R[1]; 
        case 1: return (R[2] << 8) | R[3]; 
        default: return 0; 
    }
}

//Set flags based on enum mask
void CPU::SetFlag(uint8_t mask, bool set)
{
    if (set) F |= mask; 
    else F &= ~mask; 
}

//Get flags based on enum mask
bool CPU::GetFlag(uint8_t mask)
{
    return F & mask; 
}

//Step through and run instruction

/*
    NOP         No opperation
    HLT         Halt
    LDI         Load immediate value into register
    LD          Load from 14 bit address into register
    LDIR        Load from address stored in source register into destination register
    LDIRP       Load from address stored in register pair into destination register
    ST          Store to 14 bit address from register
    STIR        Store value in register to location in source register
    STIRP       Store value in register into location in register pair
    MOV         Move from source to destination register
    ADD         Add from 2 source registers to destination register
    ADC         Add with carry
    ADDI        Add source register with 8 bit immediate value into source register
    ADDIW       Add register pair with 16 bit immediate value into source register pair
    AND         And 2 source registers into destination register
    OR          Or 2 source registers into destination register
    XOR         Xor 2 source registers into destination register
    NOT         Not source register into destination register
    OUT         Output 8 bit value
    OUTP        Output 16 bit value
    OUTA        Output Ascii value
    CPI         Compare source register to 8 bit immediate value
    JMP         Unconditional jump to memory address register
    BGT         Branch if N flag not set to memory address register
    BLT         Branch if N flag set to memory address register
    BEQ         Branch if Z flag set to memory address register
    CALL        Branch to memory address register and push program counter to the stack
    RET         Return from function poped from stack
*/
void CPU::Step()
{
    uint8_t opcode = Fetch8(); //Get opcode and switch through
    switch (opcode)
    {
        case 0x00: //NOP
            break;
        case 0x01: //HLT
            halted = true;
            break;
        case 0x02: //LDI Rd, 0xImm
        {
            MDRH = Fetch8() & 0x03; 
            MDRL = Fetch8(); 
            R[MDRH] = MDRL;
            break;
        }
        case 0x03: //LD Rd, 0xAddress
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x3F) << 8) | MDRL;
            R[((MDRH & 0xC0) >> 6)] = mem->Read(0x8000 + MAR);
            break;
        }
        case 0x04: //St Rx, 0xAddress
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x3F) << 8) | MDRL;
            mem->Write(0x8000 + MAR,R[((MDRH & 0xC0) >> 6)]);
            break;
        }
        case 0x05: //MOV Rd, Rx
        {
            MDRL = Fetch8();
            R[(MDRL & 0x0C) >> 2] = R[MDRL & 0x03];
            break;
        }
        case 0x06: //ADD Rd, Rx, Ry
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
        case 0x07: //ADC Rd, Rx, Ry
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
        case 0x08: //AND Rd, Rx, Ry
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] & R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x09: //JMP MAR
        {
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            PC = MAR;
            break;
        }
        case 0x0A: //OUT Rx
        {
            MDRL = Fetch8();
            DisplayReg(MDRL);
            break;
        }
        case 0x0B: //CPI Rx, 0xImm
        {
            MDRH = Fetch8();
            MDRL = Fetch8();
            uint8_t result = R[MDRH & 0x03] - MDRL;
            SetFlag(Z, result == 0);
            SetFlag(N, result & 0x80);
            break;
        }
        case 0x0C: //BEQ MAR
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
        case 0x0D: //BGT MAR
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
        case 0x0E: //LDIR Rd, (Rx)
        {
            MDRL = Fetch8();
            uint8_t Ra = MDRL & 0x03;
            uint8_t Rd = (MDRL & 0x0C) >> 2;
            uint16_t addr = R[Ra] + 0x8000; 
            R[Rd] = mem->Read(addr);
            break;
        }
        case 0x0F: //STIR (Rx), Ry 
        {
            MDRL = Fetch8();
            uint8_t Ra = MDRL & 0x03;
            uint8_t Rb = (MDRL & 0x0C) >> 2;
            uint16_t addr = R[Ra] + 0x8000;
            mem->Write(addr, R[Rb]);
            break;
        }
        case 0x10: //LDIRP Rd, (Rp)
        {
            MDRL = Fetch8();
            uint8_t Rd = (MDRL >> 4) & 0x0F;
            uint8_t pairId = MDRL & 0x01;
            uint16_t addr = GetPair(pairId) + 0x8000;
            R[Rd & 0x03] = mem->Read(addr); 
            break;
        }
        case 0x11: //STIRP (R0), Ry
        {
            MDRL = Fetch8();
            uint8_t Rs = (MDRL >> 4) & 0x0F;
            uint8_t pairId = MDRL & 0x01;
            uint16_t addr = GetPair(pairId) + 0x8000;
            mem->Write(addr, R[Rs & 0x03]);
            break;
        }
        case 0x12: //ADDIW Rp, 0xImm16
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
        case 0x13: //ADDI Rd, 0xImm8
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
        case 0x14: //OUTP Rp
        {
            MDRL = Fetch8();
            DisplayRegP(MDRL & 0x01);
            break;
        }
        case 0x15: //OUTA Rx
        {
            MDRL = Fetch8();
            DisplayRegAscii(MDRL);
            break;
        }
        case 0x16: //OR Rd, Rx, Ry
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] | R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x17: //Not Rd, Rx
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = ~(R[MDRL & 0x03] >> 2);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x18: //XOR Rd, Rx, Ry
        {
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] ^ R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            break;
        }
        case 0x19: //BLT MAR
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
        case 0x20: //CALL MAR
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
        case 0x21: //RET
        {
            uint8_t temp1, temp2;
            temp1 = mem->Read(++SP);
            temp2 = mem->Read(++SP);
            PC = (temp1 << 8) | temp2;
            break;
        }
        default: //Unknow opcode
            std::cerr << "Unknown opcode 0x" << std::hex << int(opcode) << "\n";
            halted = true;
            break;
    }
}