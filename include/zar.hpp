#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "Z3Dfile.hpp"
#include "utils.hpp"

class ZAR : Z3Dfile {
public:

    ZAR(path filepath, path outputDirectory) : Z3Dfile(filepath, outputDirectory) {
        char magic[4];
        file.read(magic, 4);
        if (!strncmp(magic, "ZAR\x01", 4)) {
            std::cerr << "File \"" << filepath << "\" is not a ZAR." << std::endl;
            return;
        }
    }

    void extract(void) {
        extractImpl();
    }

private:

    void extractImpl(void);
};
