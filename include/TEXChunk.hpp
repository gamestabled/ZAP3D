#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "tinyxml2.h"

#include "Z3DTexture.hpp"
#include "Z3DResource.hpp"

class TEXChunk : public Z3DResource {
public:
    class TextureHeader {
    public:
        static TextureHeader* FromBinary(std::ifstream& ins, uint32_t offset, uint32_t offsetToTextureData);
    
        inline void Save(const path outFolder) {
            texture->Save(outFolder);
        }

        tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent, path filepath);

        uint32_t length;
        uint16_t mipmapCount;
        bool isETC1;
        bool isCubemap;
        uint16_t width;
        uint16_t height;
        PicaPixelFormat picaPixelFormat;
        PicaDataType picaDataType;
        uint32_t textureDataOffset;
        std::string textureName;
        Z3DTexture* texture;
    };

    static TEXChunk* FromBinary(std::ifstream& ins, uint32_t offset, uint32_t offsetToTextureData);
    void Save(const path outFolder) override;
    tinyxml2::XMLNode* CreateXMLNode(tinyxml2::XMLNode* parent, path textureDir);

private:
    uint32_t size;
    uint32_t numTextures;
    std::vector<TextureHeader*> textures;
};

