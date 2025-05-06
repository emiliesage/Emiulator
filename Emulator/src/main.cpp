#include <iostream>
#include <fstream>
#include <cstdint>
#include "MEMORY.h"
#include "CPU.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    std::ifstream file(filename, std::ios::binary);  
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    std::vector<uint8_t> program;
    file.seekg(0, std::ios::end);  
    std::streamsize size = file.tellg();  
    file.seekg(0, std::ios::beg); 
    program.resize(size);  

    if (!file.read(reinterpret_cast<char*>(program.data()), size)) {
        std::cerr << "Error reading file!" << std::endl;
        return 1;
    }

    std::cout << "Successfully read " << program.size() << " bytes from " << filename << std::endl;

    MEM mem;
    mem.LoadProgram(program, 0);

    CPU cpu(&mem);
    cpu.Reset();

    cpu.Run();
    std::cout << "Execution complete" << std::endl;

    return 0;
}