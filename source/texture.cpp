#include <stdint.h>
#include <vector>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include "texture.hpp"

static const uint8_t SwizzleLUT[64] = { 
    0,  1,  8,  9,  2,  3, 10, 11,
    16, 17, 24, 25, 18, 19, 26, 27,
    4,  5, 12, 13,  6,  7, 14, 15,
    20, 21, 28, 29, 22, 23, 30, 31,
    32, 33, 40, 41, 34, 35, 42, 43,
    48, 49, 56, 57, 50, 51, 58, 59,
    36, 37, 44, 45, 38, 39, 46, 47,
    52, 53, 60, 61, 54, 55, 62, 63
};

static const uint8_t ETC1LUT[] = {
    0 //TODO
};

static const uint32_t BPP[] = {
    [(uint32_t)TextureFormat::RGBA8] = 32,
    [(uint32_t)TextureFormat::RGB8] = 24,
    [(uint32_t)TextureFormat::RGBA5551] = 16, 
    [(uint32_t)TextureFormat::RGB565] = 16,
    [(uint32_t)TextureFormat::RGBA4] = 16,
    [(uint32_t)TextureFormat::LA8] = 16,
    [(uint32_t)TextureFormat::HiLo8] = 16,
    [(uint32_t)TextureFormat::L8] = 8,
    [(uint32_t)TextureFormat::A8] = 8,
    [(uint32_t)TextureFormat::LA4] = 8,
    [(uint32_t)TextureFormat::L4] = 4,
    [(uint32_t)TextureFormat::A4] = 4,
    [(uint32_t)TextureFormat::ETC1] = 4,
    [(uint32_t)TextureFormat::ETC1A4] = 8,
    [(uint32_t)TextureFormat::A4NoSwap] = 4,
};

struct PicaPairHash {
public:
    std::size_t operator()(const std::pair<PicaDataType, PicaPixelFormat>& x) const {
        return std::hash<PicaDataType>()(x.first) ^ std::hash<PicaPixelFormat>()(x.second);
    }
};

TextureFormat GetPixelTextureFormat(PicaDataType dataType, PicaPixelFormat pixelFormat) {
    static const std::unordered_map<std::pair<PicaDataType, PicaPixelFormat>, TextureFormat, PicaPairHash> PicaFormatMap( {
    /* RGBA4444 */  { { PicaDataType::UnsignedShort4444, PicaPixelFormat::RGBANativeDMP },            TextureFormat::RGBA4 },
    /* RGBA5551 */  { { PicaDataType::UnsignedShort5551, PicaPixelFormat::RGBANativeDMP },            TextureFormat::RGBA5551 },
    /* RGBA8888 */  { { PicaDataType::UnsignedByte,      PicaPixelFormat::RGBANativeDMP },            TextureFormat::RGBA8 },
    /* RGB565 */    { { PicaDataType::UnsignedShort565,  PicaPixelFormat::RGBNativeDMP },             TextureFormat::RGB565 },
    /* RGB888 */    { { PicaDataType::UnsignedByte,      PicaPixelFormat::RGBNativeDMP },             TextureFormat::RGB8 },
    /* ETC1 */      { { PicaDataType::UnsignedByte,      PicaPixelFormat::ETC1RGB8NativeDMP },        TextureFormat::ETC1 },
    /* ETC1 */      { { PicaDataType(0),                 PicaPixelFormat::ETC1RGB8NativeDMP },        TextureFormat::ETC1 },
    /* ETC1_A4 */   { { PicaDataType::UnsignedByte,      PicaPixelFormat::ETC1AlphaRGB8A4NativeDMP }, TextureFormat::ETC1A4 },
    /* A8 */        { { PicaDataType::UnsignedByte,      PicaPixelFormat::AlphaNativeDMP },           TextureFormat::A8 },
    /* A4 */        { { PicaDataType::Unsigned4BitsDMP,  PicaPixelFormat::AlphaNativeDMP },           TextureFormat::A4 },
    /* L8 */        { { PicaDataType::UnsignedByte,      PicaPixelFormat::LuminanceNativeDMP },       TextureFormat::L8 },
    /* L4 */        { { PicaDataType::Unsigned4BitsDMP,  PicaPixelFormat::LuminanceNativeDMP },       TextureFormat::L4 },
    /* LA88 */      { { PicaDataType::UnsignedByte,      PicaPixelFormat::LuminanceAlphaNativeDMP },  TextureFormat::LA8 },
    /* LA44 */      { { PicaDataType::UnsignedByte44DMP, PicaPixelFormat::LuminanceAlphaNativeDMP },  TextureFormat::LA4 },
    } );

    std::pair<PicaDataType, PicaPixelFormat> key(dataType, pixelFormat);
    return PicaFormatMap.at(key);
}

std::vector<uint8_t> Texture::PicaDecodeBuffer(std::vector<uint8_t> input, uint32_t width, uint32_t height, TextureFormat format) {
    if ((format == TextureFormat::ETC1) || (format == TextureFormat::ETC1A4)) {
        return std::vector<uint8_t>(); //TODO
    }

    uint32_t increment = BPP[(uint32_t)format] / 8;
    if (increment == 0) increment = 1;

    std::vector<uint8_t> output;
    output.resize(width * height * 4);

    uint32_t iOffset = 0;

    for (uint32_t tileY = 0; tileY < height; tileY += 8) {
        for (uint32_t tileX = 0; tileX < width; tileX += 8) {
            for (uint32_t pixel = 0; pixel < 64; pixel++) {
                uint32_t X = SwizzleLUT[pixel] & 7;
                uint32_t Y = (SwizzleLUT[pixel] - X) >> 3;

                uint32_t oOffset = (tileX + X + (((tileY + Y)) * width)) * 4;

                uint8_t L, A, R, G, B;
                uint16_t value;

                switch (format) {
                    case TextureFormat::RGBA8:
                        output[oOffset + 0] = input[iOffset + 3];
                        output[oOffset + 1] = input[iOffset + 2];
                        output[oOffset + 2] = input[iOffset + 1];
                        output[oOffset + 3] = input[iOffset + 0];
                        break;

                    case TextureFormat::RGB8:
                        output[oOffset + 0] = input[iOffset + 2];
                        output[oOffset + 1] = input[iOffset + 1];
                        output[oOffset + 2] = input[iOffset + 0];
                        output[oOffset + 3] = 0xFF;
                        break;

                    case TextureFormat::RGBA5551:
                        value = input[iOffset] | (input[iOffset + 1] << 8);
                        R = ((value >>  1) & 0x1F) << 3;
                        G = ((value >>  6) & 0x1F) << 3;
                        B = ((value >> 11) & 0x1F) << 3;
                        A = (value & 1) * 0xFF;

                        output[oOffset + 0] = R | (R >> 5);
                        output[oOffset + 1] = G | (G >> 6);
                        output[oOffset + 2] = B | (B >> 5);
                        output[oOffset + 3] = A;
                        break;

                    case TextureFormat::RGB565:
                        value = input[iOffset] | (input[iOffset + 1] << 8);
                        R = ((value >>  0) & 0x1F) << 3;
                        G = ((value >>  5) & 0x3F) << 2;
                        B = ((value >> 11) & 0x1F) << 3;

                        output[oOffset + 0] = R | (R >> 5);
                        output[oOffset + 1] = G | (G >> 6);
                        output[oOffset + 2] = B | (B >> 5);
                        output[oOffset + 3] = 0xFF;
                        break;

                    case TextureFormat::RGBA4:
                        R = (input[iOffset + 1] >> 4) & 0xF;
                        G = (input[iOffset + 1]) & 0xF;
                        B = (input[iOffset + 0] >> 4) & 0xF;
                        A = (input[iOffset + 0]) & 0xF;

                        output[oOffset + 0] = R | (R << 4);
                        output[oOffset + 1] = G | (G << 4);
                        output[oOffset + 2] = B | (B << 4);
                        output[oOffset + 3] = A | (A << 4);
                        break;

                    case TextureFormat::LA8:
                        output[oOffset + 0] = input[iOffset + 1];
                        output[oOffset + 1] = input[iOffset + 1];
                        output[oOffset + 2] = input[iOffset + 1];
                        output[oOffset + 3] = input[iOffset + 0];
                        break;

                    case TextureFormat::HiLo8:
                        output[oOffset + 0] = input[iOffset + 1];
                        output[oOffset + 1] = input[iOffset + 0];
                        output[oOffset + 2] = 0;
                        output[oOffset + 3] = 0xFF;
                        break;

                    case TextureFormat::L8:
                        output[oOffset + 0] = input[iOffset];
                        output[oOffset + 1] = input[iOffset];
                        output[oOffset + 2] = input[iOffset];
                        output[oOffset + 3] = input[iOffset];
                        break;

                    case TextureFormat::A8:
                        output[oOffset + 0] = 0xFF;
                        output[oOffset + 1] = 0xFF;
                        output[oOffset + 2] = 0xFF;
                        output[oOffset + 3] = input[iOffset];
                        break;

                    case TextureFormat::LA4:
                        output[oOffset + 0] = (uint8_t)((input[iOffset] >> 4) | (input[iOffset] & 0xF0));
                        output[oOffset + 1] = (uint8_t)((input[iOffset] >> 4) | (input[iOffset] & 0xF0));
                        output[oOffset + 2] = (uint8_t)((input[iOffset] >> 4) | (input[iOffset] & 0xF0));
                        output[oOffset + 3] = (uint8_t)((input[iOffset] << 4) | (input[iOffset] & 0x0F));
                        break;

                    case TextureFormat::L4:
                        L = (input[iOffset >> 1] >> ((iOffset & 1) << 2)) & 0xF;
                        output[oOffset + 0] = (L << 4) | L;
                        output[oOffset + 1] = (L << 4) | L;
                        output[oOffset + 2] = (L << 4) | L;
                        output[oOffset + 3] = 0xFF;
                        break;

                    case TextureFormat::A4:
                        A = (input[iOffset >> 1] >> ((iOffset & 1) << 2)) & 0xF;
                        output[oOffset + 0] = 0xFF;
                        output[oOffset + 1] = 0xFF;
                        output[oOffset + 2] = 0xFF;
                        output[oOffset + 3] = (A << 4) | A;
                        break;

                    default:
                        break;
                }
                iOffset += increment;
            }
        }
    }
    return output;
}

std::vector<uint8_t> Texture::DecodeBuffer(std::vector<uint8_t> input, uint32_t width, uint32_t height, TextureFormat format) {
    if ((format == TextureFormat::ETC1) || (format == TextureFormat::ETC1A4)) {
        return std::vector<uint8_t>(); //TODO
    }

    uint32_t increment = BPP[(uint32_t)format] / 8;
    if (increment == 0) increment = 1;

    std::vector<uint8_t> output;
    output.resize(width * height * 4);

    uint32_t iOffset = 0;

    for (uint32_t Y = 0; Y < height; Y++) {
        for (uint32_t X = 0; X < width; X++) {

            uint32_t oOffset = ((Y * width) + X) * 4;

            uint8_t L, A, R, G, B;
            uint16_t value;

            switch (format) {
                case TextureFormat::RGBA8:
                    output[oOffset + 0] = input[iOffset + 3];
                    output[oOffset + 1] = input[iOffset + 2];
                    output[oOffset + 2] = input[iOffset + 1];
                    output[oOffset + 3] = input[iOffset + 0];
                    break;

                case TextureFormat::RGB8:
                    output[oOffset + 0] = input[iOffset + 2];
                    output[oOffset + 1] = input[iOffset + 1];
                    output[oOffset + 2] = input[iOffset + 0];
                    output[oOffset + 3] = 0xFF;
                    break;

                case TextureFormat::RGBA5551:
                    value = input[iOffset] | (input[iOffset + 1] << 8);
                    R = ((value >>  1) & 0x1F) << 3;
                    G = ((value >>  6) & 0x1F) << 3;
                    B = ((value >> 11) & 0x1F) << 3;
                    A = (value & 1) * 0xFF;

                    output[oOffset + 0] = R | (R >> 5);
                    output[oOffset + 1] = G | (G >> 6);
                    output[oOffset + 2] = B | (B >> 5);
                    output[oOffset + 3] = A;
                    break;

                case TextureFormat::RGB565:
                    value = input[iOffset] | (input[iOffset + 1] << 8);
                    R = ((value >>  0) & 0x1F) << 3;
                    G = ((value >>  5) & 0x3F) << 2;
                    B = ((value >> 11) & 0x1F) << 3;

                    output[oOffset + 0] = R | (R >> 5);
                    output[oOffset + 1] = G | (G >> 6);
                    output[oOffset + 2] = B | (B >> 5);
                    output[oOffset + 3] = 0xFF;
                    break;

                case TextureFormat::RGBA4:
                    R = (input[iOffset + 1] >> 4) & 0xF;
                    G = (input[iOffset + 1]) & 0xF;
                    B = (input[iOffset + 0] >> 4) & 0xF;
                    A = (input[iOffset + 0]) & 0xF;

                    output[oOffset + 0] = R | (R << 4);
                    output[oOffset + 1] = G | (G << 4);
                    output[oOffset + 2] = B | (B << 4);
                    output[oOffset + 3] = A | (A << 4);
                    break;

                case TextureFormat::LA8:
                    output[oOffset + 0] = input[iOffset + 1];
                    output[oOffset + 1] = input[iOffset + 1];
                    output[oOffset + 2] = input[iOffset + 1];
                    output[oOffset + 3] = input[iOffset + 0];
                    break;

                case TextureFormat::HiLo8:
                    output[oOffset + 0] = input[iOffset + 1];
                    output[oOffset + 1] = input[iOffset + 0];
                    output[oOffset + 2] = 0;
                    output[oOffset + 3] = 0xFF;
                    break;

                case TextureFormat::L8:
                    output[oOffset + 0] = input[iOffset];
                    output[oOffset + 1] = input[iOffset];
                    output[oOffset + 2] = input[iOffset];
                    output[oOffset + 3] = input[iOffset];
                    break;

                case TextureFormat::A8:
                    output[oOffset + 0] = 0xFF;
                    output[oOffset + 1] = 0xFF;
                    output[oOffset + 2] = 0xFF;
                    output[oOffset + 3] = input[iOffset];
                    break;

                case TextureFormat::LA4:
                    output[oOffset + 0] = (uint8_t)((input[iOffset] >> 4) | (input[iOffset] & 0xF0));
                    output[oOffset + 1] = (uint8_t)((input[iOffset] >> 4) | (input[iOffset] & 0xF0));
                    output[oOffset + 2] = (uint8_t)((input[iOffset] >> 4) | (input[iOffset] & 0xF0));
                    output[oOffset + 3] = (uint8_t)((input[iOffset] << 4) | (input[iOffset] & 0x0F));
                    break;

                case TextureFormat::L4:
                    L = (input[iOffset >> 1] >> ((iOffset & 1) << 2)) & 0xF;
                    output[oOffset + 0] = (L << 4) | L;
                    output[oOffset + 1] = (L << 4) | L;
                    output[oOffset + 2] = (L << 4) | L;
                    output[oOffset + 3] = 0xFF;
                    break;

                case TextureFormat::A4:
                    A = (input[iOffset >> 1] >> ((iOffset & 1) << 2)) & 0xF;
                    output[oOffset + 0] = 0xFF;
                    output[oOffset + 1] = 0xFF;
                    output[oOffset + 2] = 0xFF;
                    output[oOffset + 3] = (A << 4) | A;
                    break;

                case TextureFormat::A4NoSwap:
                    if (iOffset & 1) {
                        A = input[iOffset >> 1] & 0xF;
                    } else {
                        A = (input[iOffset >> 1] >> 0x4) & 0xF;
                    }
                    output[oOffset + 0] = 0xFF;
                    output[oOffset + 1] = 0xFF;
                    output[oOffset + 2] = 0xFF;
                    output[oOffset + 3] = (A << 4) | A;
                    break;

                default:
                    break;
            }
            iOffset += increment;
        }
    }
    return output;
}

void Texture::PrepareBitmap(void) {
    if (isPica) {
        preparedBitmap = PicaDecodeBuffer(rawData, width, height, format);
    } else {
        preparedBitmap = DecodeBuffer(rawData, width, height, format);
    }
}

Texture* Texture::FromBinary(TextureFormat format, std::vector<uint8_t> rawData, std::string name, int width, int height, bool isPica) {
    Texture* tex = new Texture();

    tex->width = width;
    tex->height = height;
    tex->format = format;
    tex->isPica = isPica;
    tex->name = name;
    tex->rawData = rawData;

    tex->PrepareBitmap();

    return tex;
}

void Texture::Save(const path outDir, const std::string name) {
    path outfilePath = outDir / name;
    std::experimental::filesystem::create_directories(outfilePath.parent_path());

    switch (format) {
        case TextureFormat::RGBA8: outfilePath += ".rgba8"; break;
        case TextureFormat::RGB8:  outfilePath += ".rgb8";  break;
        case TextureFormat::RGBA5551: outfilePath += ".rgba5551"; break;
        case TextureFormat::RGB565: outfilePath += ".rgb565"; break;
        case TextureFormat::RGBA4: outfilePath += ".rgba4"; break;
        case TextureFormat::LA8: outfilePath += ".la8"; break;
        case TextureFormat::HiLo8: outfilePath += ".hilo8"; break;
        case TextureFormat::L8: outfilePath += ".l8"; break;
        case TextureFormat::A8: outfilePath += ".a8"; break;
        case TextureFormat::LA4: outfilePath += ".la4"; break;
        case TextureFormat::L4: outfilePath += ".l4"; break;
        case TextureFormat::A4: outfilePath += ".a4"; break;
        case TextureFormat::ETC1: outfilePath += ".etc1"; break;
        case TextureFormat::ETC1A4: outfilePath += ".etc1a4"; break;
        case TextureFormat::A4NoSwap: outfilePath += ".a4noswap"; break;
    }
    outfilePath += ".png";
    stbi_write_png(outfilePath.c_str(), width, height, 4, preparedBitmap.data(), width * 4);
}
