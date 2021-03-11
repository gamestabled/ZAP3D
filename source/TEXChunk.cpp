#include <iostream>
#include <cstring>

#include "utils.hpp"
#include "StringHelper.hpp"

#include "TEXChunk.hpp"

using namespace std;
using namespace tinyxml2;

TEXChunk::TextureHeader* TEXChunk::TextureHeader::FromBinary(ifstream& ins, uint32_t offset, uint32_t offsetToTextureData) {
    ins.seekg(offset);
    Reader reader(ins);

    TEXChunk::TextureHeader* header = new TEXChunk::TextureHeader;

    header->length = reader.ReadU32();
    header->mipmapCount = reader.ReadU16();
    header->isETC1 = (bool)reader.ReadU8();
    header->isCubemap = (bool)reader.ReadU8();
    header->width = reader.ReadU16();
    header->height = reader.ReadU16();
    header->picaPixelFormat = (PicaPixelFormat)reader.ReadU16();
    header->picaDataType = (PicaDataType)reader.ReadU16();
    header->textureDataOffset = reader.ReadU32();
    char nameBuf[20] = { 0 };
    ins.read(nameBuf, 16);
    header->textureName = nameBuf;

    vector<uint8_t> rawTexData;
    ins.seekg(offsetToTextureData + header->textureDataOffset);
    for (uint32_t i = 0; i < header->length; i++) {
        rawTexData.push_back(reader.ReadU8());
    }

    TextureFormat texFormat = GetPixelTextureFormat(header->picaDataType, header->picaPixelFormat);
    header->texture = Z3DTexture::FromBinary(texFormat, rawTexData, header->textureName, header->width, header->height, header->picaDataType != PicaDataType::None);

    return header;
}

XMLNode* TEXChunk::TextureHeader::CreateXMLNode(XMLNode* parent, path textureDir) {
    XMLElement* textureElement = parent->GetDocument()->NewElement("Texture");
    textureElement->SetAttribute("MipmapCount", mipmapCount);
    textureElement->SetAttribute("ETC1", isETC1);
    textureElement->SetAttribute("Cubemap", isCubemap);
    textureElement->SetAttribute("Width", width);
    textureElement->SetAttribute("Height", height);
    textureElement->SetAttribute("PicaPixelFormat", (int)picaPixelFormat);
    textureElement->SetAttribute("PicaDataType", (int)picaDataType);
    textureElement->SetAttribute("Name", textureName.c_str());
    textureElement->SetText((textureDir / texture->GetName()).c_str());
    parent->InsertEndChild(textureElement);
    return textureElement;
}

TEXChunk* TEXChunk::FromBinary(ifstream& ins, uint32_t offset, uint32_t offsetToTextureData) {
    ins.seekg(offset);
    {
        char texMagic[8] = { 0 };
        ins.read(texMagic, 4);
        if (strncmp(texMagic, "tex ", 4)) {
            std::cerr << "Error parsing TEX chunk" << std::endl;
            return nullptr;
        }
    }

    TEXChunk* chunk = new TEXChunk();

    chunk->size = ReadU32FromFile(ins);
    chunk->numTextures = ReadU32FromFile(ins);

    for (uint32_t i = 0; i < chunk->numTextures; i++) {
        TextureHeader* header = TextureHeader::FromBinary(ins, offset + 0xC + (i * 0x24), offsetToTextureData);
        chunk->textures.push_back(header);
        if (header->textureName != "") {
            header->texture->SetName(header->textureName);
        } else {
            header->texture->SetName(StringHelper::Sprintf("tex_%d", i));
        }
    }
    return chunk;
}

void TEXChunk::Save(const path outFolder) {
    for (auto& tex : textures) {
        tex->Save(outFolder);
    }
}

XMLNode* TEXChunk::CreateXMLNode(XMLNode* parent, path textureDir) {
    XMLNode* texRoot = parent->GetDocument()->NewElement("TEXTURES");
    parent->InsertEndChild(texRoot);
    for (auto& header : textures) {
        header->CreateXMLNode(texRoot, textureDir);
    }
    return texRoot;
}
