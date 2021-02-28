#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"

class ZAR {
public:
    using path = std::experimental::filesystem::path;

    ZAR(path filepath, path outputDirectory) : file(filepath, std::ios::binary) {
        if (file.fail()) {
            std::cerr << "Failed to open \"" << filepath << "\"." << std::endl;
            return;
        }

        char magic[4];
        file.read(magic, 4);
        if (magic != std::string("ZAR\x01")) {
            std::cerr << "File \"" << filepath << "\" is not a ZAR." << std::endl;
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

    void extractImpl(void);

    path name;
    std::ifstream file;
};
