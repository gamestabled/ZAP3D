#pragma once

#include <string>
#include "resourceTypes.hpp"

class Z3DCfile;

class Z3Dresource {
public:
    virtual std::string GetSourceTypeName(void) = 0;
    virtual std::string GetSourceOutputCode(void) = 0;
    virtual uint32_t Size(void) = 0;

    Z3DCfile* GetParentFile(void) {
        return parent;
    }

protected:
    std::string name;
    bool isArray;
    Z3DCfile* parent;
};

enum class DeclarationAlignment {
    None,
    Align4,
    Align8,
    Align16
};

enum class DeclarationPadding {
    None,
    Pad4,
    Pad8,
    Pad16
};

class Declaration {
public:
    Declaration(DeclarationAlignment alignment, DeclarationPadding padding, uint32_t size, std::string varType, std::string varName, bool isArray, std::string body);
    Declaration(DeclarationAlignment alignment, uint32_t size, std::string varType, std::string varName, bool isArray, int arrayItemCount, std::string body);

    DeclarationAlignment alignment;
    DeclarationPadding padding;
    int size;
    std::string body;
    std::string varType;
    std::string varName;
    bool isArray;
    int arrayItemCount;
};
