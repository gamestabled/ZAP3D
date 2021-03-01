#include <string>
#include <vector>
#include <experimental/filesystem>
#include <sstream>
#include <iostream>

#include "qbf.hpp"
#include "utils.hpp"
#include "texture.hpp"

const static int BUF_SIZE = 4096;

struct CharacterMetadata {
    uint16_t character;
    uint16_t glyphId;
    uint8_t unk_04;
    uint8_t unk_05;
    uint16_t unk_06;
};

void QBF::extractImpl(void) {
    file.seekg(4);
    numCharacters = ReadU16();
    unk_06 = ReadU16();
    unk_08 = ReadU32();
    bitsPerPixel = ReadU8();
    glyphWidth = ReadU8();
    glyphHeight = ReadU8();
    unk_0F = ReadU8();

    const uint32_t bitmapSectionOffset = 0x10 + (numCharacters * 8);
    const uint32_t bitmapSize = ((bitsPerPixel * glyphHeight * glyphWidth) / 8);

    for (uint32_t charID = 0; charID < numCharacters; ++charID) {
        CharacterMetadata currentChar;

        file.seekg(0x10 + (charID * 8));
        currentChar.character = ReadU16();
        currentChar.glyphId = ReadU16();
        currentChar.unk_04 = ReadU8();
        currentChar.unk_05 = ReadU8();
        currentChar.unk_06 = ReadU16();

        file.seekg(bitmapSectionOffset + (currentChar.glyphId * bitmapSize));
        std::vector<uint8_t> rawBitmapData;
        for (uint32_t rawByte = 0; rawByte < bitmapSize; ++rawByte) {
            rawBitmapData.push_back(ReadU8());
        }
        
        TextureType texType = TextureType::Error;
        if (bitsPerPixel == 4) {
            texType = TextureType::Grayscale4bpp;
        } else if (bitsPerPixel == 8) {
            texType = TextureType::Grayscale8bpp;
        }

        Texture* tex = Texture::FromBinary(texType, rawBitmapData, "", glyphWidth, glyphHeight);
        std::stringstream textureFilename;
        textureFilename << "tex_char_" << currentChar.glyphId;
        std::cout << textureFilename.str() << std::endl;
        tex->Save(name, textureFilename.str());
    }
}
