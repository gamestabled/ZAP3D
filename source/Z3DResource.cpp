#include "Z3DResource.hpp"

using namespace std;

Z3DResource::Z3DResource() {
    name = "";
    sourceOutput = "";
    rawData = vector<uint8_t>();
    rawDataIndex = 0;
}

void Z3DResource::Save(const path outFolder) {
    (void)outFolder;
}

string Z3DResource::GetName(void) {
    return name;
}

void Z3DResource::SetName(string inName) {
    name = std::move(inName);
}

string Z3DResource::GetSourceOutputCode(const string& prefix) {
    (void)prefix;
    return "";
}

string Z3DResource::GetSourceOutputHeader(const string& prefix) {
    (void)prefix;
    return "";
}

void Z3DResource::ParseRawData(void) {

}

string Z3DResource::GetSourceTypeName(void) {
    return "";
}

int Z3DResource::GetRawDataSize(void) {
    return 0;
}
