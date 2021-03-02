#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <map>

#include "utils.hpp"
#include "Z3Dresource.hpp"

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

class Z3DCfile {
public:
    using path = std::experimental::filesystem::path;

    Z3DCfile(path name) : name(name) {
        declarations.clear();
    }

    Declaration* AddDeclaration(uint32_t offset, DeclarationAlignment alignment, DeclarationPadding padding, uint32_t size, std::string varType, std::string varName, std::string body) {
        Declaration* dec = new Declaration(alignment, padding, size, varType, varName, false, body);
        declarations[offset] = dec;
        return dec;
    }

    Declaration* AddDeclarationArray(uint32_t offset, DeclarationAlignment alignment, uint32_t size, std::string varType, std::string varName, int arrayItemCnt, std::string body) {
        Declaration* dec = new Declaration(alignment, size, varType, varName, true, arrayItemCnt, body);
        declarations[offset] = dec;
        return dec;
    }

    void BuildSourceFile(path outputDir) {
        GenerateSourceFiles(outputDir);
    }

private:
    void GenerateSourceFiles(path outputDir);
    std::string ProcessDeclarations(void);
    std::string ProcessExterns(void);

protected:
    path name;
    std::vector<Z3Dresource*> resources;
    std::map<uint32_t, Declaration*> declarations;
};
