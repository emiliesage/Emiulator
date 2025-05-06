
#include <iostream>
#include "CPU.h"

//Program memory starts at 0x8000
//Data Memory starts at 0x4000
//Stack pointer starts at 0x100

void CPU::DisplayReg(uint8_t reg)
{
    //std::cout << "R" << int(reg) << " = 0x" << std::hex << int(R[(reg & 0x03)]) << std::endl;
    std::cout << int(R[(reg & 0x03)]) << std::endl;
}

void CPU::DisplayRegAscii(uint8_t reg)
{
    //std::cout << "R" << int(reg) << " = 0x" << std::hex << int(R[(reg & 0x03)]) << std::endl;
    std::cout << static_cast<char>(R[(reg & 0x03)]);
}

void CPU::DisplayRegP(uint8_t pairId)
{
    //std::cout << "R" << int(reg) << " = 0x" << std::hex << int(R[(reg & 0x03)]) << std::endl;
    switch (pairId) {
        case 0: std::cout << ((R[0] << 8) | R[1]) << std::endl; break; // R01
        case 1: std::cout << ((R[2] << 8) | R[3]) << std::endl; break;// R23
    }
}

CPU::CPU(MEM* mem) : mem(mem)
{
    Reset(); //Constructor resets cpu
}

void CPU::Reset()
{
    //Set all data in registers to be 0
    F = 0;
    R.fill(0);
    MDRH = MDRL = 0;
    MAR = 0;
    PC = 0x0000; //Program counter begins at 32kb
    SP = 0xFE00; //Stack pointer begins after Zero Page
}

uint8_t CPU::Fetch8()
{
    return mem->Read(PC++); //Read from memory
}

void CPU::Run()
{
    //If not halted step through program
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
        case 0: return (R[0] << 8) | R[1]; // R01
        case 1: return (R[2] << 8) | R[3]; // R23
        default: return 0; // or assert/error
    }
}

void CPU::SetFlag(uint8_t mask, bool set)
{
    if (set) F |= mask; //Set flag register with mask
    else F &= ~mask; //Remove flag register with mask
}

bool CPU::GetFlag(uint8_t mask)
{
    return F & mask; //return flag register with mask
}

void CPU::Step()
{
    uint8_t opcode = Fetch8();
    switch (opcode)
    {
        case 0x00: //NOP
            //std::cout << "NOP" << std::endl;
            break;
        case 0x01: //HLT
            //std::cout << "HLT" << std::endl;
            halted = true;
            break;
        case 0x02: //LDI R 0xImm
        {
            //std::cout << "LDI ";
            MDRH = Fetch8() & 0x03; //Get register
            MDRL = Fetch8(); //Get immediate data
            R[MDRH] = MDRL;
            //std::cout << "0x" << std::hex << int(R[MDRH]) << std::endl;
            break;
        }
        case 0x03: //LD Rx 0xAddress
        {
            //std::cout << "LD ";
            MDRH = Fetch8(); //Get register
            MDRL = Fetch8(); //Get immediate data
            MAR = ((MDRH & 0x3F) << 8) | MDRL;
            R[((MDRH & 0xC0) >> 6)] = mem->Read(0x8000 + MAR);
            //std::cout << "0x" << std::hex << int(R[((MDRH & 0xC0) >> 6)]) << std::endl;
            break;
        }
        case 0x04: //ST Rx 0xAddress
        {
            //std::cout << "ST Rd 0xAddress";
            MDRH = Fetch8(); //Get register
            MDRL = Fetch8(); //Get immediate data
            MAR = ((MDRH & 0x3F) << 8) | MDRL;
            mem->Write(0x8000 + MAR,R[((MDRH & 0xC0) >> 6)]);
            //std::cout << "0x" << std::hex << int(R[((MDRH & 0xC0) >> 6)]) << std::endl;
            break;
        }
        case 0x05: //MOV Rd Ra
        {
            //std::cout << "MOV ";
            MDRL = Fetch8();
            R[(MDRL & 0x0C) >> 2] = R[MDRL & 0x03];
            //std::cout << "R" << ((MDRL & 0x0C) >> 2) << " R" << (MDRL & 0x03) << std::endl;
            break;
        }
        case 0x06: //ADD Rd Ra Rb 
        {
            //std::cout << "ADD ";
            MDRH = Fetch8();
            uint16_t result = R[(MDRH & 0x0C) >> 2] + R[(MDRH & 0x03)];
            SetFlag(V,((R[(MDRH & 0x0C) >> 2] ^ ~R[(MDRH & 0x03)]) & (R[(MDRH & 0x0C) >> 2] ^ result) & 0x80) != 0);
            SetFlag(N,result & 0x80);
            SetFlag(Z,(result & 0xFF) == 0);
            SetFlag(C,result > 0xFF);
            R[(MDRH & 0x30) >> 4] = static_cast<uint8_t>(result);
            //std::cout << "R" << ((MDRH & 0x30) >> 4) << " R" << ((MDRH & 0x0C) >> 2) << " R" << (MDRH & 0x03) << std::endl;
            break;
        }
        case 0x07: //ADC Rd Ra Rb  
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
        case 0x08: // AND Rd Ra Rb
        {
            //std::cout << "NAND ";
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] & R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            //std::cout << "R" << ((MDRL & 0x30) >> 4) << " R" << ((MDRL & 0x0C) >> 2) << " R" << (MDRL & 0x03) << std::endl;
            break;
        }
        case 0x09: //JMP MAR + 0x8000
        {
            //std::cout << "JMP ";
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            PC = MAR;
            //std::cout << "0x" << std::hex << PC << std::endl;
            break;
        }
        case 0x0A: //OUT
        {
            MDRL = Fetch8();
            DisplayReg(MDRL);
            break;
        }
        case 0x0B: //CPI
        {
            //std::cout << "CPI ";
            MDRH = Fetch8();
            MDRL = Fetch8();
            uint8_t result = R[MDRH & 0x03] - MDRL;
            SetFlag(Z, result == 0);
            SetFlag(N, result & 0x80);
            //std::cout << "R" << (MDRH & 0x03) << " 0x" << std::hex << int(MDRL) << std::endl;
            //std::cout << "Z = " << GetFlag(Z) << " N = " << GetFlag(N) << std::endl;
            break;
        }
        case 0x0C: //BEQ
        {
            //std::cout << "BEQ" << std::endl;
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            if (GetFlag(Z))
            { 
                PC = MAR;
            }
            //std::cout << "Z = " << GetFlag(Z) << std::endl;
            break;
        }
        case 0x0D: // BGT
        {
            //std::cout << "BGT" << std::endl;
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            if (GetFlag(N) != 1)
            { 
                PC = MAR;
            }
            //std::cout << "N = " << GetFlag(N) << std::endl;
            break;
        }
        case 0x0E: // LDIR Rd, (Ra)
        {
            MDRL = Fetch8();
            uint8_t Ra = MDRL & 0x03;
            uint8_t Rd = (MDRL & 0x0C) >> 2;
            uint16_t addr = R[Ra] + 0x8000; // Assume program memory space
            R[Rd] = mem->Read(addr);
            break;
        }
        case 0x0F: // STIR (Ra), Rb
        {
            MDRL = Fetch8();
            uint8_t Ra = MDRL & 0x03;
            uint8_t Rb = (MDRL & 0x0C) >> 2;
            uint16_t addr = R[Ra] + 0x8000;
            mem->Write(addr, R[Rb]);
            break;
        }
        case 0x10: // LDIRP Rd, (Rp)
        {
            MDRL = Fetch8();
            uint8_t Rd = (MDRL >> 4) & 0x0F;
            uint8_t pairId = MDRL & 0x01;
            uint16_t addr = GetPair(pairId) + 0x8000;
            R[Rd & 0x03] = mem->Read(addr); // Only low 2 bits needed for R index
            break;
        }
        case 0x11: // STIRP (Rp), Rs
        {
            MDRL = Fetch8();
            uint8_t Rs = (MDRL >> 4) & 0x0F;
            uint8_t pairId = MDRL & 0x01;
            uint16_t addr = GetPair(pairId) + 0x8000;
            mem->Write(addr, R[Rs & 0x03]);
            break;
        }
        case 0x12: // ADDIW Rp, Imm16
        {
            MDRL = Fetch8();                  // Contains pair ID
            uint8_t pairId = MDRL & 0x01;     // Only need 2 bits
            uint8_t immHigh = Fetch8();        // Low byte of immediate
            uint8_t immLow = Fetch8();       // High byte of immediate
            uint16_t imm = (immHigh << 8) | immLow;
            
            uint16_t regVal = GetPair(pairId);
            uint32_t result = regVal + imm;

            SetPair(pairId, static_cast<uint16_t>(result));

            // Optionally set flags (Z, C, N)
            SetFlag(Z, (result & 0xFFFF) == 0);
            SetFlag(N, result & 0x8000);
            SetFlag(C, result > 0xFFFF);
            break;
        }
        case 0x13: // ADDI Rd, #Imm8
        {
            MDRH = Fetch8();                  // contains Rd in bits 1:0
            MDRL = Fetch8();                  // 8-bit immediate value
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
        case 0x15: //OUTA Ra
        {
            MDRL = Fetch8();
            DisplayRegAscii(MDRL);
            break;
        }
        case 0x16: // OR Rd Ra Rb
        {
            //std::cout << "OR ";
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] | R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            //std::cout << "R" << ((MDRL & 0x30) >> 4) << " R" << ((MDRL & 0x0C) >> 2) << " R" << (MDRL & 0x03) << std::endl;
            break;
        }
        case 0x17: // NOT Rd Ra 
        {
            //std::cout << "NOT ";
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = ~(R[MDRL & 0x03] >> 2);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            //std::cout << "R" << ((MDRL & 0x30) >> 4) << " R" << ((MDRL & 0x0C) >> 2) << " R" << (MDRL & 0x03) << std::endl;
            break;
        }
        case 0x18: // XOR Rd Ra Rb
        {
            //std::cout << "XOR ";
            MDRL = Fetch8();
            R[(MDRL & 0x30) >> 4] = (R[MDRL & 0x03] ^ R[(MDRL & 0x0C) >> 2]);
            SetFlag(Z, R[(MDRL & 0x30) >> 4] == 0);
            SetFlag(N, R[(MDRL & 0x30) >> 4] & 0x80);
            //std::cout << "R" << ((MDRL & 0x30) >> 4) << " R" << ((MDRL & 0x0C) >> 2) << " R" << (MDRL & 0x03) << std::endl;
            break;
        }
        case 0x19: // BLT
        {
            //std::cout << "BGT" << std::endl;
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;
            if (GetFlag(N) == 1)
            { 
                PC = MAR;
            }
            //std::cout << "N = " << GetFlag(N) << std::endl;
            break;
        }
        case 0x20: //CALL MAR + 0x8000
        {
            //std::cout << "JMP ";
            MDRH = Fetch8(); 
            MDRL = Fetch8(); 
            MAR = ((MDRH & 0x7F) << 8) | MDRL;

            uint16_t temp = PC;
            mem->Write(SP--,(temp & 0xFF));
            mem->Write(SP--,((temp >> 8) & 0xFF));
            PC = MAR;
            //std::cout << "0x" << std::hex << PC << std::endl;
            break;
        }
        case 0x21: //CALL MAR + 0x8000
        {
            //std::cout << "JMP ";

            uint8_t temp1, temp2;
            temp1 = mem->Read(++SP);
            temp2 = mem->Read(++SP);
            PC = (temp1 << 8) | temp2;
            //std::cout << "0x" << std::hex << PC << std::endl;
            break;
        }
        default:
            std::cerr << "Unknown opcode 0x" << std::hex << int(opcode) << "\n";
            halted = true;
            break;
    }
    //std::cerr << "PC: 0x" << std::hex << PC << std::endl;
}