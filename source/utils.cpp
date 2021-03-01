#include <cstdint>
#include <iostream>
#include <fstream>

#include "utils.hpp"

uint32_t ReadU32FromFile(std::ifstream& ifile) {
    uint8_t buf[8] = { 0 };
    uint32_t number = 0;

    ifile.read((char*)buf, 4);
    for (uint32_t i = 0; i < 4; ++i) {
        number |= (((uint32_t)buf[i]) << (i * 8));
    }

    return number;
}

uint16_t ReadU16FromFile(std::ifstream& ifile) {
    uint8_t buf[8] = { 0 };
    uint16_t number = 0;

    ifile.read((char*)buf, 2);
    for (uint32_t i = 0; i < 2; ++i) {
        number |= (((uint32_t)buf[i]) << (i * 8));
    }

    return number;
}

uint8_t ReadU8FromFile(std::ifstream& ifile) {
    uint8_t buf[8] = { 0 };
    uint16_t number = 0;

    ifile.read((char*)buf, 1);
    number |= (uint32_t)buf[0];

    return number;
}
