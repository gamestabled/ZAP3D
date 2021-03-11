#include <iostream>
#include <cstring>

#include "utils.hpp"

#include "CTXBChunk.hpp"

using namespace std;
using namespace tinyxml2;

CTXBChunk* CTXBChunk::FromBinary(ifstream& ins, uint32_t offset) {
    ins.seekg(offset);
    {
        char ctxbMagic[8] = { 0 };
        ins.read(ctxbMagic, 4);
        if (strncmp(ctxbMagic, "ctxb", 4)) {
            std::cerr << "Error parsing CTXB chunk" << std::endl;
            return nullptr;
        }
    }

    CTXBChunk* chunk = new CTXBChunk();
    Reader reader(ins);
    chunk->size = reader.ReadU32();
    chunk->numTEXChunks = reader.ReadU32();
    chunk->unk_0C = reader.ReadU32();
    chunk->offsetToTEXChunk = reader.ReadU32();
    chunk->offsetToTextureData = reader.ReadU32();

    chunk->texChunk = TEXChunk::FromBinary(ins, chunk->offsetToTEXChunk, chunk->offsetToTextureData);
    chunk->texChunk->SetName(chunk->name);
    return chunk;
}

XMLNode* CTXBChunk::CreateXMLNode(XMLNode* parent) {
    XMLNode* ctxbRoot = parent->GetDocument()->NewElement("CTXB");
    parent->InsertEndChild(ctxbRoot);
    texChunk->CreateXMLNode(ctxbRoot, name);
    return ctxbRoot;
}

void CTXBChunk::Save(const path outFolder) {
    path ctxbDir = outFolder / name;
    experimental::filesystem::create_directories(ctxbDir);
    texChunk->Save(ctxbDir);

    XMLDocument* ctxbDoc = new XMLDocument();
    CreateXMLNode(ctxbDoc);
    ctxbDoc->SaveFile((outFolder / (name + ".xml")).c_str());
}
