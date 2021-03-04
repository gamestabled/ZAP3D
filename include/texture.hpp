#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

#include "utils.hpp"

enum class PicaDataType : uint {
    Byte = 0x1400,
    UnsignedByte = 0x1401,
    Short = 0x1402,
    UnsignedShort = 0x1403,
    Int = 0x1404,
    UnsignedInt = 0x1405,
    Float = 0x1406,
    UnsignedByte44DMP = 0x6760,
    Unsigned4BitsDMP = 0x6761,
    UnsignedShort4444 = 0x8033,
    UnsignedShort5551 = 0x8034,
    UnsignedShort565 = 0x8363
};

enum class PicaPixelFormat : uint {
    RGBANativeDMP = 0x6752,
    RGBNativeDMP = 0x6754,
    AlphaNativeDMP = 0x6756,
    LuminanceNativeDMP = 0x6757,
    LuminanceAlphaNativeDMP = 0x6758,
    ETC1RGB8NativeDMP = 0x675A,
    ETC1AlphaRGB8A4NativeDMP = 0x675B
};

enum class TextureFormat : uint {
    RGBA8,
    RGB8,
    RGBA5551,
    RGB565,
    RGBA4,
    LA8,
    HiLo8,
    L8,
    A8,
    LA4,
    L4,
    A4,
    ETC1,
    ETC1A4,
    A4NoSwap,
};

TextureFormat GetPixelTextureFormat(PicaDataType dataType, PicaPixelFormat pixelFormat);

class Texture {
public:
    using path = std::experimental::filesystem::path;

    Texture() {};

    static Texture* FromBinary(TextureFormat inType, std::vector<uint8_t> inRawData, std::string inName, int inWidth, int inHeight, bool isPica);
    static std::vector<uint8_t> PicaDecodeBuffer(std::vector<uint8_t> input, uint32_t width, uint32_t height, TextureFormat format);
    static std::vector<uint8_t> DecodeBuffer(std::vector<uint8_t> input, uint32_t width, uint32_t height, TextureFormat format);

    void Save(const path outDir, const std::string name);

private:
    void PrepareBitmap(void);

    int width = 0;
    int height = 0;
    TextureFormat format;
    bool isPica;
    std::string name;
    std::vector<uint8_t> rawData;
    std::vector<uint8_t> preparedBitmap;
};
