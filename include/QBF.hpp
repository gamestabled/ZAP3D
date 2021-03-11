#pragma once

#include <cstring>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"
#include "Z3DResource.hpp"
#include "Z3DTexture.hpp"

class QBF : public Z3DResource {
public:
    class QBFEntry {
    public:
        static QBFEntry* FromBinary(std::ifstream& ins, uint32_t offset, uint32_t offsetToBitmapSection, int glyphWidth, int glyphHeight, uint32_t bitmapSize, uint32_t bitsPerPixel);
        inline void Save(const path outFolder) {
            tex->Save(outFolder);
        }
        tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent, path bitmapDir);

        uint16_t character;
        uint16_t glyphID;
        uint8_t unk_04;
        uint8_t unk_05;
        uint16_t unk_06;
        Z3DTexture* tex;
    };

    static QBF* FromBinary(std::ifstream& ins, uint32_t offset);
    void Save(const path outFolder) override;
    tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent);

private:
    uint16_t numCharacters;
    uint16_t unk_06;
    uint32_t unk_08;
    uint8_t bitsPerPixel;
    uint8_t glyphWidth;
    uint8_t glyphHeight;
    uint8_t unk_0F;
    std::vector<QBFEntry*> entries;
};
