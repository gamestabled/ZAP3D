#include <string>
#include <iostream>
#include <cerrno>
#include <cstring>

#include "Z3Dfile.hpp"
#include "Z3Dresource.hpp"
#include "StringHelper.hpp"

void Z3DCfile::GenerateSourceFiles(path outputDir){
    std::string sourceOutput = "";
    std::string headerOutput = "";

    path sourceOutputFile = outputDir / (outputDir.stem().string() + ".c");
    path headerOutputFile = outputDir / (outputDir.stem().string() + ".h");

    std::cout << "Extracting " << outputDir << std::endl;

    sourceOutput += StringHelper::Sprintf("#include \"%s\"\n\n", headerOutputFile.filename().c_str());
    sourceOutput += ProcessDeclarations();
    std::experimental::filesystem::create_directories(sourceOutputFile.parent_path());
    std::ofstream sourceOutfile(sourceOutputFile);
    if (!sourceOutfile) {
        std::cout << std::strerror(errno) << std::endl;
    }
    sourceOutfile.write(sourceOutput.c_str(), sourceOutput.size());

    headerOutput += ProcessExterns();
    std::experimental::filesystem::create_directories(headerOutputFile.parent_path());
    std::ofstream headerOutfile(headerOutputFile);
    if (!headerOutfile) {
        std::cout << std::strerror(errno) << std::endl;
    }
    headerOutfile.write(headerOutput.c_str(), headerOutput.size());

}

std::string Z3DCfile::ProcessDeclarations(void) {
    std::string output = "";

    if (declarations.size() == 0) {
        return output;
    }

    for (auto decl : declarations) {
        if (decl.second->isArray) {
            if (decl.second->arrayItemCount == 0) {
                output += StringHelper::Sprintf("%s %s[] = {\n", decl.second->varType.c_str(), decl.second->varName.c_str());
            } else {
                output += StringHelper::Sprintf("%s %s[%i] = {\n", decl.second->varType.c_str(), decl.second->varName.c_str(), decl.second->arrayItemCount);
            }
            output += decl.second->body + "};";
        } else {
            output += StringHelper::Sprintf("%s %s = { ", decl.second->varType.c_str(), decl.second->varName.c_str());
            output += decl.second->body + " };";
        }

        output += "\n\n";
    }

    output += "\n";

    return output;
}

std::string Z3DCfile::ProcessExterns(void) {
    std::string output = "";

    if (declarations.size() == 0) {
        return output;
    }

    for (auto decl : declarations) {
        if (decl.second->varType != "") {
            if (decl.second->isArray) {
                if (decl.second->arrayItemCount == 0) {
                    output += StringHelper::Sprintf("extern %s %s[];\n", decl.second->varType.c_str(), decl.second->varName.c_str());
                } else {
                    output += StringHelper::Sprintf("extern %s %s[%i];\n", decl.second->varType.c_str(), decl.second->varName.c_str(), decl.second->arrayItemCount);
                }
            } else {
                output += StringHelper::Sprintf("extern %s %s;\n", decl.second->varType.c_str(), decl.second->varName.c_str());
            }
        }
    }

    output += "\n";

    return output;
}
