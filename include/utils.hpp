#pragma once

#include <cstdint>

uint32_t ReadU32FromFile(std::ifstream& ifile);
uint16_t ReadU16FromFile(std::ifstream& ifile);
uint8_t ReadU8FromFile(std::ifstream& ifile);

class Reader {
public:
    Reader(std::ifstream& file) : file(file) {}

    inline uint32_t ReadU32(void) {
        return ReadU32FromFile(file);
    }

    inline uint16_t ReadU16(void) {
        return ReadU16FromFile(file);
    }

    inline uint8_t ReadU8(void) {
        return ReadU8FromFile(file);
    }

private:
    std::ifstream& file;
};
