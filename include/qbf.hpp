#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"

class QBF {
public:
    using path = std::experimental::filesystem::path;

    QBF(path filepath, path outputDirectory) : file(filepath, std::ios::binary) {
        if (file.fail()) {
            std::cerr << "Failed to open \"" << filepath << "\"." << std::endl;
            return;
        }

        char magic[4];
        file.read(magic, 4);
        if (magic != std::string("QBF1")) {
            std::cerr << "File \"" << filepath << "\" is not a QBF." << std::endl;
            return;
        }

        name = outputDirectory / filepath.stem();
    }

    void extract(void) {
        extractImpl();
    }

private:
    uint32_t ReadU32(void) {
        return ReadU32FromFile(file);
    }

    uint16_t ReadU16(void) {
        return ReadU16FromFile(file);
    }

    uint8_t ReadU8(void) {
        return ReadU8FromFile(file);
    }

    void extractImpl(void);

    uint16_t numCharacters;
    uint16_t unk_06;
    uint32_t unk_08;
    uint8_t bitsPerPixel;
    uint8_t glyphWidth;
    uint8_t glyphHeight;
    uint8_t unk_0F;

    path name;
    std::ifstream file;
};
