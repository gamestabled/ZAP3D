#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"

enum class TextureType {
    RGBA32bpp,
    RGBA16bpp,
    Palette4bpp,
    Palette8bpp,
    Grayscale4bpp,
    Grayscale8bpp,
    GrayscaleAlpha4bpp,
    GrayscaleAlpha8bpp,
    GrayscaleAlpha16bpp,
    Error
};

class Texture {
public:
    using path = std::experimental::filesystem::path;

    Texture() {};

    static Texture* FromBinary(TextureType inType, std::vector<uint8_t> inRawData, std::string inName, int inWidth, int inHeight);

    void Save(const path outDir, const std::string name);

private:
    void PrepareBitmap(void);

    void PrepareBitmapGrayScale4(void);
    void PrepareBitmapGrayScale8(void);

    uint8_t* bmpRGB = nullptr;
    uint8_t* bmpRGBA = nullptr;
    int width = 0;
    int height = 0;
    TextureType type = TextureType::Error;
    std::string name;
    std::vector<uint8_t> rawData;
};
