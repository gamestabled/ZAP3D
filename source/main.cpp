#include <iostream>
#include <string>
#include <experimental/filesystem>

#include "tinyxml2.h"

#include "CTXBChunk.hpp"
#include "QBF.hpp"

using path = std::experimental::filesystem::path;
using namespace std;
using namespace tinyxml2;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage:\n";
        std::cerr << "    ZAP3D <input file> <output directory>\n";
        return 1;
    }

    path inputName = argv[1];
    path outputDir = argv[2];

    if (inputName.extension() == ".ctxb") {
        std::ifstream ins(inputName);
        CTXBChunk* ctxb = CTXBChunk::FromBinary(ins, 0);
        if (ctxb) {
            ctxb->SetName(inputName.stem());
            ctxb->Save(outputDir);
        }
    } else if (inputName.extension() == ".qbf") {
        std::ifstream ins(inputName);
        QBF* qbf = QBF::FromBinary(ins, 0);
        if (qbf) {
            qbf->SetName(inputName.stem());
            qbf->Save(outputDir);
        }
    } else {
        std::cout << "Filetype not yet supported" << std::endl;
    }

    return 0;
}
