#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"
#include "Z3Dfile.hpp"
#include "Z3Dresource.hpp"
#include "StringHelper.hpp"

class QCLEntry : Z3Dresource {
public:
    QCLEntry(uint32_t index, uint8_t r, uint8_t g, uint8_t b, uint a) : 
        index(index), r(r), g(g), b(b), a(a) {}

    std::string GetSourceTypeName(void) {
        return "TextColor";
    }

    std::string GetSourceOutputCode(void) {
        return StringHelper::Sprintf("{ %i, { %i, %i, %i, %i } }", index, r, g, b, a);
    }

    uint32_t Size(void) {
        return sizeof(TextColor);
    }

private:
    uint32_t index;
    uint8_t r, g, b, a;
};

class QCLResource : public Z3Dresource {
public:
using path = std::experimental::filesystem::path;

    QCLResource(){}

    QCLResource(path name) {
        isArray = true;
        parent = new Z3DCfile(name);
    }

    std::string GetSourceTypeName(void) {
        return "TextColor";
    }

    std::string GetSourceOutputCode(void) {
        std::string output = "";

        for (QCLEntry entry : entries) {
            output += (StringHelper::Sprintf("    %s,", entry.GetSourceOutputCode().c_str()) + "\n");
        }
        if (parent != nullptr) {
            parent->AddDeclarationArray(0, DeclarationAlignment::None, Size(), GetSourceTypeName(), "", NumEntries(), output);
        }
        
        return "";
    }

    uint32_t Size(void) {
        return entries.size() * sizeof(TextColor);
    }

    void AddEntry(QCLEntry entry) {
        entries.push_back(entry);
    }

    uint32_t NumEntries(void) {
        return entries.size();
    }

private:
    std::vector<QCLEntry> entries;
};

class QCL : Z3Dfile {
public:

    QCL(path filepath, path outputDirectory) : Z3Dfile(filepath, outputDirectory) {
        entries = QCLResource(name);
        outfile = entries.GetParentFile();
    }

    void extract(void) {
        extractImpl();
    }

private:
    void extractImpl(void);

    QCLResource entries;
    Z3DCfile* outfile;
};
