#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <experimental/filesystem>
#include <algorithm>

#include "ctxb.hpp"
#include "utils.hpp"

void CTXB::extractImpl(void) {
    file.seekg(4);
    const uint32_t size             = ReadU32(); (void)size;
    const uint16_t numChunks        = ReadU32(); (void)numChunks;
    const uint16_t unk_0C           = ReadU32(); (void)unk_0C;
    const uint32_t offsetToTexChunk = ReadU32();
    const uint32_t offsetToTexData  = ReadU32();

    file.seekg(offsetToTexChunk);
    char texMagic[8] = { 0 };
    file.read(texMagic, 4);
    if (strncmp(texMagic, "tex ", 4)) {
        std::cerr << "Error parsing CTXB file" << std::endl;
        return;
    }

    const uint32_t texChunkSize = ReadU32(); (void)texChunkSize;
    const uint32_t texCount     = ReadU32();

    for (uint32_t i = 0; i < texCount; ++i) {
        ReadCTXBTextureHeader(offsetToTexChunk + 0xC + (i * 0x24));
        std::vector<uint8_t> rawTexData;

        file.seekg(offsetToTexData + textures[i]->dataOffset);
        for (uint32_t j = 0; j < textures[i]->dataLength; ++j) {
            rawTexData.push_back(ReadU8());
        }

        TextureFormat texType = GetPixelTextureFormat(textures[i]->dataType, textures[i]->pixelFormat);
        Texture* tex = Texture::FromBinary(texType, rawTexData, textures[i]->name, textures[i]->width, textures[i]->height, true);
        std::stringstream textureFilename;
        if (textures[i]->name != "") {
            textureFilename << textures[i]->name;
        } else {
            textureFilename << "tex_" << i;
        }
        std::cout << textureFilename.str() << std::endl;
        tex->Save(name, textureFilename.str());
    }
}
