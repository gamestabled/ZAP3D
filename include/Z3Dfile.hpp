#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"

class Z3Dfile {
public:
    using path = std::experimental::filesystem::path;

    Z3Dfile(path filepath, path outputDirectory) : file(filepath, std::ios::binary) {
        if (file.fail()) {
            std::cerr << "Failed to open \"" << filepath << "\"." << std::endl;
            return;
        }
        name = outputDirectory / filepath.stem();
    }

    virtual void extract(void) = 0;

protected:
    uint32_t ReadU32(void) {
        return ReadU32FromFile(file);
    }

    uint16_t ReadU16(void) {
        return ReadU16FromFile(file);
    }

    uint8_t ReadU8(void) {
        return ReadU8FromFile(file);
    }

    path name;
    std::ifstream file;
};
