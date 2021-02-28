#include "zar.hpp"
#include "utils.hpp"
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <experimental/filesystem>
#include <algorithm>

const static int BUF_SIZE = 4096;

struct ZARFileType {
    uint32_t numFiles;
    uint32_t offsetToFilesList;
    uint32_t offsetToTypeName;
    uint32_t unknown_0C;
    std::string name;
    std::vector<uint32_t> filesList;
};

struct FileInZAR {
    std::string filetype;
    std::string filename;
    uint32_t size;
    uint32_t offsetToFileData;
};

void ZAR::extractImpl(void) {
    file.seekg(4);
    const uint32_t size                        = ReadU32(); (void)size;
    const uint16_t numFileTypes                = ReadU16();
    const uint16_t numFiles                    = ReadU16(); (void)numFiles;
    const uint32_t offsetToFiletypesSection    = ReadU32();
    const uint32_t offsetToFileMetadataSection = ReadU32();
    const uint32_t offsetToDataSection         = ReadU32();

    for (uint32_t i = 0; i < numFileTypes; ++i) {
        ZARFileType currentFileType;

        file.seekg(offsetToFiletypesSection + (i * 0x10));
        currentFileType.numFiles          = ReadU32();
        currentFileType.offsetToFilesList = ReadU32();
        currentFileType.offsetToTypeName  = ReadU32();
        currentFileType.unknown_0C        = ReadU32();

        file.seekg(currentFileType.offsetToTypeName);
        std::getline(file, currentFileType.name, '\0');

        if (currentFileType.offsetToFilesList != 0xFFFFFFFF) {
            file.seekg(currentFileType.offsetToFilesList);
            for (uint32_t j = 0; j < currentFileType.numFiles; ++j) {
                currentFileType.filesList.push_back(ReadU32());
            }
        }

        for (uint32_t fileNum : currentFileType.filesList) {
            FileInZAR currentFile;

            currentFile.filetype = currentFileType.name;
            file.seekg(offsetToFileMetadataSection + (fileNum * 8));
            currentFile.size = ReadU32();
            uint32_t offsetToFilename = ReadU32();

            file.seekg(offsetToFilename);
            std::getline(file, currentFile.filename, '\0');
            std::replace(currentFile.filename.begin(), currentFile.filename.end(), '\\', '/');

            file.seekg(offsetToDataSection + (fileNum * 4));
            currentFile.offsetToFileData = ReadU32();

            file.seekg(currentFile.offsetToFileData);
            char buf[BUF_SIZE];
            path outfilePath = name / currentFile.filename;
            std::cout << "Extracting " << outfilePath << std::endl;

            std::experimental::filesystem::create_directories(outfilePath.parent_path());
            std::ofstream outfile(outfilePath, std::ios::binary | std::ios::trunc);
            if (!outfile) {
                std::cout << std::strerror(errno) << std::endl;
            }

            uint32_t read = 0;
            uint32_t amountToRead = 0;
            do {
                if (read + BUF_SIZE > currentFile.size) {
                    amountToRead = currentFile.size - read;
                } else {
                    amountToRead = BUF_SIZE;
                }
                file.read(buf, amountToRead);
                outfile.write(buf, file.gcount());
                read += file.gcount();
            } while (file.gcount() > 0);
            outfile.close();
        }
    }
}
