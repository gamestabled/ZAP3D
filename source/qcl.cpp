#include <string>
#include <vector>
#include <experimental/filesystem>
#include <sstream>
#include <iostream>

#include "qcl.hpp"
#include "utils.hpp"
#include "StringHelper.hpp"
#include "Z3Dresource.hpp"

void QCL::extractImpl(void) {
    file.seekg(0);

    while(file.tellg() != 0x800) {
        uint32_t index = ReadU32();
        uint8_t r = ReadU8();
        uint8_t g = ReadU8();
        uint8_t b = ReadU8();
        uint8_t a = ReadU8();
        entries.AddEntry(QCLEntry(index, r, g, b, a));
    }

    entries.GetSourceOutputCode();
    outfile->BuildSourceFile(name);
}
