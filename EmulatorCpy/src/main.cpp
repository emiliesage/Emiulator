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
    std::ifstream file(filename, std::ios::binary);  // Open in binary mode
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    // Read the binary file into a vector of uint8_t (byte-by-byte)
    std::vector<uint8_t> program;
    file.seekg(0, std::ios::end);  // Go to the end to get the file size
    std::streamsize size = file.tellg();  // Get the size of the file
    file.seekg(0, std::ios::beg);  // Go back to the beginning of the file
    program.resize(size);  // Resize the vector to the file size

    // Read the file into the program vector
    if (!file.read(reinterpret_cast<char*>(program.data()), size)) {
        std::cerr << "Error reading file!" << std::endl;
        return 1;
    }

    std::cout << "Successfully read " << program.size() << " bytes from " << filename << std::endl;

    // Initialize memory and load the program into memory starting at address 0
    MEM mem;
    mem.LoadProgram(program, 0);

    // Initialize CPU with memory
    CPU cpu(&mem);
    cpu.Reset();

    // Uncomment and use for stepping through
    /* for (int i = 0; i < 10; i++) {
        cpu.Step();
    } */

    // Run the CPU
    cpu.Run();
    //cpu.DisplayReg(2);
    std::cout << "Execution complete" << std::endl;

    return 0;
}