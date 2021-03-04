#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "Z3Dfile.hpp"
#include "utils.hpp"
#include "texture.hpp"

class CTXBTexture {
public:
    uint32_t dataLength;
    uint16_t unk_04;
    uint16_t unk_06;
    uint16_t width;
    uint16_t height;
    PicaPixelFormat pixelFormat;
    PicaDataType dataType;
    uint32_t dataOffset;
    std::string name;
    Texture tex;
};

class CTXB : Z3Dfile {
public:

    CTXB(path filepath, path outputDirectory) : Z3Dfile(filepath, outputDirectory) {
        char magic[4];
        file.read(magic, 4);
        if (magic != std::string("ctxb")) {
            std::cerr << "File \"" << filepath << "\" is not a CTXB." << std::endl;
            return;
        }
    }

    void ReadCTXBTextureHeader(uint32_t offset) {
        file.seekg(offset);
        CTXBTexture* ctex = new CTXBTexture;
        ctex->dataLength = ReadU32();
        ctex->unk_04 = ReadU16();
        ctex->unk_06 = ReadU16();
        ctex->width = ReadU16();
        ctex->height = ReadU16();
        ctex->pixelFormat = (PicaPixelFormat)ReadU16();
        ctex->dataType = (PicaDataType)ReadU16();
        ctex->dataOffset = ReadU32();
        char nameBuf[16];
        file.read(nameBuf, 16);
        ctex->name = nameBuf;
        textures.push_back(ctex);
    }

    void extract(void) {
        extractImpl();
    }

private:

    void extractImpl(void);

    std::vector<CTXBTexture*> textures;
};