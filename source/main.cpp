#include <iostream>
#include <string>
#include <experimental/filesystem>

#include "zar.hpp"

using path = std::experimental::filesystem::path;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage:\n";
        std::cerr << "    ZAP3D <input file> <output directory>\n";
        return 1;
    }

    path inputName = argv[1];
    path outputDir = argv[2];

    if (inputName.extension() == ".zar") {
        ZAR ZARFile(inputName, outputDir);
        ZARFile.extract();
    } else {
        std::cout << "Filetype not yet supported" << std::endl;
    }

    return 0;
}
