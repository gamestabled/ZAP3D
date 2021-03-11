#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"
#include "StringHelper.hpp"
#include "Z3DResource.hpp"

class QCL : public Z3DResource {
public:
    class QCLEntry : public Z3DResource {
    public:
        static QCLEntry* FromBinary(std::ifstream& ins, uint32_t offset);
        tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent);

    private:
        uint32_t index;
        uint8_t r, g, b, a;
    };

    static QCL* FromBinary(std::ifstream& ins, uint32_t offset);
    tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent);
    void Save(const path outFolder) override;

private:
    std::vector<QCLEntry*> entries;
};
