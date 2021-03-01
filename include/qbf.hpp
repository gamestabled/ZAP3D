#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "Z3Dfile.hpp"
#include "utils.hpp"

class QBF : Z3Dfile {
public:

    QBF(path filepath, path outputDirectory) : Z3Dfile(filepath, outputDirectory) {
        char magic[4];
        file.read(magic, 4);
        if (magic != std::string("QBF1")) {
            std::cerr << "File \"" << filepath << "\" is not a QBF." << std::endl;
            return;
        }
    }

    void extract(void) {
        extractImpl();
    }

private:

    void extractImpl(void);

    uint16_t numCharacters;
    uint16_t unk_06;
    uint32_t unk_08;
    uint8_t bitsPerPixel;
    uint8_t glyphWidth;
    uint8_t glyphHeight;
    uint8_t unk_0F;
};
