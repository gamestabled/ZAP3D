#include <string>
#include <vector>
#include <experimental/filesystem>
#include <sstream>
#include <iostream>

#include "utils.hpp"

#include "QCL.hpp"

using namespace std;
using namespace tinyxml2;

QCL::QCLEntry* QCL::QCLEntry::FromBinary(ifstream& ins, uint32_t offset) {
    ins.seekg(offset);

    QCLEntry* entry = new QCLEntry();
    Reader reader(ins);
    entry->index = reader.ReadU32();
    entry->r = reader.ReadU8();
    entry->g = reader.ReadU8();
    entry->b = reader.ReadU8();
    entry->a = reader.ReadU8();
    return entry;
}

XMLNode* QCL::QCLEntry::CreateXMLNode(XMLNode* parent) {
    XMLElement* qclElement = parent->GetDocument()->NewElement("Color");
    qclElement->SetAttribute("Index", index);
    qclElement->SetAttribute("r", r);
    qclElement->SetAttribute("g", g);
    qclElement->SetAttribute("b", b);
    qclElement->SetAttribute("a", a);
    parent->InsertEndChild(qclElement);
    return qclElement;
}

QCL* QCL::FromBinary(ifstream& ins, uint32_t offset) {
    ins.seekg(offset);
    
    QCL* qcl = new QCL();
    uint32_t index = 0;

    do {
        QCLEntry* entry = QCLEntry::FromBinary(ins, offset + (index * 0x8));
        qcl->entries.push_back(entry);
        ins.seekg(offset + ((index + 1) * 8));
        index = ReadU32FromFile(ins);
    } while (index != 0);

    return qcl;
}

XMLNode* QCL::CreateXMLNode(XMLNode* parent) {
    XMLNode* qclRoot = parent->GetDocument()->NewElement("QCL");
    parent->InsertEndChild(qclRoot);
    for (auto entry : entries) {
        entry->CreateXMLNode(qclRoot);
    }
    return qclRoot;
}

void QCL::Save(const path outFolder) {
    XMLDocument* qclDoc = new XMLDocument();
    CreateXMLNode(qclDoc);
    qclDoc->SaveFile((outFolder / (name + ".xml")).c_str());    
}
