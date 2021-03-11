#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "tinyxml2.h"

#include "TEXChunk.hpp"
#include "Z3DResource.hpp"

class CTXBChunk : public Z3DResource {
public:
    static CTXBChunk* FromBinary(std::ifstream& ins, uint32_t offset);
    void Save(const path outFolder) override;
    tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent);

private:
    uint32_t size;
    uint32_t numTEXChunks; // Is this always 1?
    uint32_t unk_0C;
    uint32_t offsetToTEXChunk;
    uint32_t offsetToTextureData;
    TEXChunk* texChunk;
};

