#include <string>
#include <vector>
#include <experimental/filesystem>
#include <sstream>
#include <iostream>

#include "utils.hpp"
#include "Z3DTexture.hpp"

#include "QBF.hpp"

using namespace std;
using namespace tinyxml2;

QBF::QBFEntry* QBF::QBFEntry::FromBinary(ifstream& ins, uint32_t offset, uint32_t offsetToBitmapSection, int glyphWidth, int glyphHeight, uint32_t bitmapSize, uint32_t bitsPerPixel) {
    ins.seekg(offset);

    QBFEntry* entry = new QBFEntry();
    Reader reader(ins);

    entry->character = reader.ReadU16();
    entry->glyphID = reader.ReadU16();
    entry->unk_04 = reader.ReadU8();
    entry->unk_05 = reader.ReadU8();
    entry->unk_06 = reader.ReadU16();

    vector<uint8_t> rawBitmapData;
    ins.seekg(offsetToBitmapSection + (entry->glyphID * bitmapSize));
    for (uint32_t rawByte = 0; rawByte < bitmapSize; rawByte++) {
        rawBitmapData.push_back(reader.ReadU8());
    }

    TextureFormat texFormat;
    if (bitsPerPixel == 4) {
        texFormat = TextureFormat::A4NoSwap;
    } else if (bitsPerPixel == 8) {
        texFormat = TextureFormat::LA4;
    }

    std::stringstream textureFilename;
    textureFilename << "tex_char_" << entry->glyphID;
    std::cout << textureFilename.str() << std::endl;
    entry->tex = Z3DTexture::FromBinary(texFormat, rawBitmapData, textureFilename.str(), glyphWidth, glyphHeight, false);

    return entry;
}

XMLNode* QBF::QBFEntry::CreateXMLNode(XMLNode* parent, path bitmapDir) {
    XMLElement* qbfElement = parent->GetDocument()->NewElement("Character");
    qbfElement->SetAttribute("char", character);
    qbfElement->SetAttribute("glyphID", glyphID);
    qbfElement->SetAttribute("unk_04", unk_04);
    qbfElement->SetAttribute("unk_05", unk_05);
    qbfElement->SetAttribute("unk_06", unk_06);
    qbfElement->SetText((bitmapDir / tex->GetName()).c_str());
    parent->InsertEndChild(qbfElement);
    return qbfElement;
}

QBF* QBF::FromBinary(std::ifstream& ins, uint32_t offset) {
    ins.seekg(offset);
    {
        char qbfMagic[8] = { 0 };
        ins.read(qbfMagic, 4);
        if (strncmp(qbfMagic, "QBF1", 4)) {
            std::cerr << "Error parsing qbf" << std::endl;
            return nullptr;
        }
    }

    QBF* qbf = new QBF();
    Reader reader(ins);
    qbf->numCharacters = reader.ReadU16();
    qbf->unk_06 = reader.ReadU16();
    qbf->unk_08 = reader.ReadU32();
    qbf->bitsPerPixel = reader.ReadU8();
    qbf->glyphWidth = reader.ReadU8();
    qbf->glyphHeight = reader.ReadU8();
    qbf->unk_0F = reader.ReadU8();

    const uint32_t bitmapSectionOffset = 0x10 + (qbf->numCharacters * 8);
    const uint32_t bitmapSize = ((qbf->bitsPerPixel * qbf->glyphHeight * qbf->glyphWidth) / 8);

    for (uint32_t charID = 0; charID < qbf->numCharacters; charID++) {
        uint32_t charOffset = 0x10 + (charID * 8);
        qbf->entries.push_back(QBFEntry::FromBinary(ins, charOffset, bitmapSectionOffset, qbf->glyphWidth, qbf->glyphHeight, bitmapSize, qbf->bitsPerPixel));
    }
    return qbf;
}

XMLNode* QBF::CreateXMLNode(XMLNode* parent) {
    XMLNode* qbfRoot = parent->GetDocument()->NewElement("QBF");
    parent->InsertEndChild(qbfRoot);
    for (auto entry : entries) {
        entry->CreateXMLNode(qbfRoot, name);
    }
    return qbfRoot;
}

void QBF::Save(const path outFolder) {
    path qbfDir = outFolder / name;
    experimental::filesystem::create_directories(qbfDir);
    for (auto& entry : entries) {
        entry->Save(qbfDir);
    }

    XMLDocument* qbfDoc = new XMLDocument();
    CreateXMLNode(qbfDoc);
    qbfDoc->SaveFile((outFolder / (name + ".xml")).c_str());
}
