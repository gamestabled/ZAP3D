#pragma once

#include <string>
#include <experimental/filesystem>

#include "resourceTypes.hpp"
#include "tinyxml2.h"

class Z3DResource {
public:
    using path = std::experimental::filesystem::path;

    Z3DResource();
    virtual void Save(const path outFolder);
    std::string GetName(void);
    void SetName(std::string name);
    virtual std::string GetSourceOutputCode(const std::string& prefix);
    virtual std::string GetSourceOutputHeader(const std::string& prefix);
    virtual void ParseRawData(void);
    virtual std::string GetSourceTypeName(void);
    virtual int GetRawDataSize(void);
protected:
    std::string name;
    std::vector<uint8_t> rawData;
    uint32_t rawDataIndex;
    std::string sourceOutput;
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
    using path = std::experimental::filesystem::path;
public:
    Declaration(DeclarationAlignment alignment, DeclarationPadding padding, uint32_t size, std::string varType, std::string varName, bool isArray, std::string body);
    Declaration(DeclarationAlignment alignment, uint32_t size, std::string varType, std::string varName, bool isArray, int arrayItemCount, std::string body);

    DeclarationAlignment alignment;
    DeclarationPadding padding;
    int size;
    std::string preBody;
    std::string body;
    std::string rightBody;
    std::string postBody;
    std::string preComment;
    std::string postComment;
    std::string varType;
    std::string varName;
    path includePath;
    bool isArray;
    int arrayItemCount;
};

