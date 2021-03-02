#include "Z3Dresource.hpp"

Declaration::Declaration(DeclarationAlignment alignment, DeclarationPadding padding, uint32_t size, std::string varType, std::string varName, bool isArray, std::string body) :
    alignment(alignment), padding(padding), size(size), body(body), varType(varType), varName(varName), isArray(isArray), arrayItemCount(0) {}

Declaration::Declaration(DeclarationAlignment alignment, uint32_t size, std::string varType, std::string varName, bool isArray, int arrayItemCount, std::string body) :
    alignment(alignment), padding(DeclarationPadding::None), size(size), body(body), varType(varType), varName(varName), isArray(isArray), arrayItemCount(arrayItemCount) {}
