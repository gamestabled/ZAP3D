#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <map>

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

static const int ETC1LUT[][4] = {
    { 2, 8, -2, -8 },
    { 5, 17, -5, -17 },
    { 9, 29, -9, -29 },
    { 13, 42, -13, -42 },
    { 18, 60, -18, -60 },
    { 24, 80, -24, -80 },
    { 33, 106, -33, -106 },
    { 47, 183, -47, -183 },
};

static const uint32_t XT[] = {
    0, 4, 0, 4
};

static const uint32_t YT[] = {
    0, 0, 4, 4
};

static std::map<TextureFormat, uint32_t> BPP {
    { TextureFormat::RGBA8, 32 },
    { TextureFormat::RGB8, 24 },
    { TextureFormat::RGBA5551, 16 },
    { TextureFormat::RGB565, 16 },
    { TextureFormat::RGBA4, 16 },
    { TextureFormat::LA8, 16 },
    { TextureFormat::HiLo8, 16 },
    { TextureFormat::L8, 8 },
    { TextureFormat::A8, 8 },
    { TextureFormat::LA4, 8 },
    { TextureFormat::L4, 4 },
    { TextureFormat::A4, 4 },
    { TextureFormat::ETC1, 4 },
    { TextureFormat::ETC1A4, 8 },
    { TextureFormat::A4NoSwap, 4 },
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
    /* ETC1_A4 */   { { PicaDataType(0),                 PicaPixelFormat::ETC1AlphaRGB8A4NativeDMP }, TextureFormat::ETC1A4 },
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

struct Color {
    uint8_t R, G, B;
};

static uint8_t Saturate(int value) {
    if (value > 255) return 255;
    if (value < 0) return 0;
    return (uint8_t)value;
}

static Color ETC1Pixel(uint32_t R, uint32_t G, uint32_t B, uint32_t X, uint32_t Y, uint32_t block, uint32_t table) {
    uint32_t index = X * 4 + Y;
    uint32_t MSB = block << 1;

    int pixel = index < 8 ? ETC1LUT[table][((block >> (index + 24)) & 1) + ((MSB >> (index + 8)) & 2)]
                          : ETC1LUT[table][((block >> (index +  8)) & 1) + ((MSB >> (index - 8)) & 2)];

    R = Saturate((int)(R + pixel));
    G = Saturate((int)(G + pixel));
    B = Saturate((int)(B + pixel));

    return { (uint8_t)R, (uint8_t)G, (uint8_t)B };
}

static std::vector<uint8_t> ETC1Tile(uint64_t block) {
    uint32_t blockLow  = (uint32_t)(block >> 32);
    uint32_t blockHigh = (uint32_t)(block >>  0);

    bool flip = ((blockHigh & 0x1000000) != 0);
    bool diff = ((blockHigh & 0x2000000) != 0);

    uint32_t R1, G1, B1;
    uint32_t R2, G2, B2;

    if (diff) {
        B1 = (blockHigh & 0x0000F8) >> 0;
        G1 = (blockHigh & 0x00F800) >> 8;
        R1 = (blockHigh & 0xF80000) >> 16;

        B2 = (uint32_t)((int8_t)(B1 >> 3) + ((int8_t)((blockHigh & 0x000007) <<  5) >> 5));
        G2 = (uint32_t)((int8_t)(G1 >> 3) + ((int8_t)((blockHigh & 0x000700) >>  3) >> 5));
        R2 = (uint32_t)((int8_t)(R1 >> 3) + ((int8_t)((blockHigh & 0x070000) >> 11) >> 5));

        B1 |= B1 >> 5;
        G1 |= G1 >> 5;
        R1 |= R1 >> 5;

        B2 = (B2 << 3) | (B2 >> 2);
        G2 = (G2 << 3) | (G2 >> 2);
        R2 = (R2 << 3) | (R2 >> 2);
    } else {
        B1 = (blockHigh & 0x0000F0) >> 0;
        G1 = (blockHigh & 0x00F000) >> 8;
        R1 = (blockHigh & 0xF00000) >> 16;

        B2 = (blockHigh & 0x00000F) << 4;
        G2 = (blockHigh & 0x000F00) >> 4;
        R2 = (blockHigh & 0x0F0000) >> 12;

        B1 |= B1 >> 4;
        G1 |= G1 >> 4;
        R1 |= R1 >> 4;

        B2 |= B2 >> 4;
        G2 |= G2 >> 4;
        R2 |= R2 >> 4;
    }

    uint32_t table1 = (blockHigh >> 29) & 7;
    uint32_t table2 = (blockHigh >> 26) & 7;

    std::vector<uint8_t> output;
    output.resize(4 * 4 * 4);

    if (!flip) {
        for (uint32_t Y = 0; Y < 4; Y++) {
            for (uint32_t X = 0; X < 2; X++) {
                Color Color1 = ETC1Pixel(R1, G1, B1, X + 0, Y, blockLow, table1);
                Color Color2 = ETC1Pixel(R2, G2, B2, X + 2, Y, blockLow, table2);

                uint32_t offset1 = (Y * 4 + X) * 4;

                // output[Offset1 + 0] = Color1.B;
                // output[Offset1 + 1] = Color1.G;
                // output[Offset1 + 2] = Color1.R;

                output[offset1 + 0] = Color1.R;
                output[offset1 + 1] = Color1.G;
                output[offset1 + 2] = Color1.B;

                uint32_t offset2 = (Y * 4 + X + 2) * 4;

                // output[Offset2 + 0] = Color2.B;
                // output[Offset2 + 1] = Color2.G;
                // output[Offset2 + 2] = Color2.R;

                output[offset2 + 0] = Color2.R;
                output[offset2 + 1] = Color2.G;
                output[offset2 + 2] = Color2.B;
            }
        }
    } else {
        for (uint32_t Y = 0; Y < 2; Y++) {
            for (uint32_t X = 0; X < 4; X++) {
                Color Color1 = ETC1Pixel(R1, G1, B1, X, Y + 0, blockLow, table1);
                Color Color2 = ETC1Pixel(R2, G2, B2, X, Y + 2, blockLow, table2);

                uint32_t offset1 = (Y * 4 + X) * 4;

                // output[offset1 + 0] = Color1.B;
                // output[offset1 + 1] = Color1.G;
                // output[offset1 + 2] = Color1.R;

                output[offset1 + 0] = Color1.R;
                output[offset1 + 1] = Color1.G;
                output[offset1 + 2] = Color1.B;

                uint32_t offset2 = ((Y + 2) * 4 + X) * 4;

                // output[offset2 + 0] = Color2.B;
                // output[offset2 + 1] = Color2.G;
                // output[offset2 + 2] = Color2.R;

                output[offset2 + 0] = Color2.R;
                output[offset2 + 1] = Color2.G;
                output[offset2 + 2] = Color2.B;
            }
        }
    }
    return output;
}

uint64_t Swap64(uint64_t value) {
    value = ((value & 0xffffffff00000000) >> 32) | ((value & 0x00000000ffffffff) << 32);
    value = ((value & 0xffff0000ffff0000) >> 16) | ((value & 0x0000ffff0000ffff) << 16);
    value = ((value & 0xff00ff00ff00ff00) >>  8) | ((value & 0x00ff00ff00ff00ff) <<  8);
    return value;
}

std::vector<uint8_t> ETC1Decompress(std::vector<uint8_t> input, uint32_t width, uint32_t height, bool alpha) {
    std::vector<uint8_t> output;
    output.resize(width * height * 4);
    uint32_t iOffset = 0;

    for (uint32_t tileY = 0; tileY < height; tileY += 8) {
        for (uint32_t tileX = 0; tileX < width; tileX += 8) {
            for (uint32_t t = 0; t < 4; t++) {

                uint64_t alphaBlock = 0;
                if (alpha) {
                    for (uint32_t i = 0; i < 8; i++) {
                        alphaBlock |= (input[iOffset] << (i * 8));
                        iOffset++;
                    }
                } else {
                    alphaBlock = 0xFFFFFFFFFFFFFFFFul;
                }

                uint64_t colorBlock = 0;
                for (int i = 0; i < 8; i++) {
                    colorBlock |= (input[iOffset] << (i * 8));
                }
                colorBlock = Swap64(colorBlock);

                std::vector<uint8_t> tile = ETC1Tile(colorBlock);
                uint32_t tileOffset = 0;

                for (uint32_t PY = YT[t]; PY < (YT[t] + 4); PY++) {
                    for (uint32_t PX = XT[t]; PX < (XT[t] + 4); ++PX) {
                        uint32_t oOffset = ((height - 1 - (tileY + PY)) * width + tileX + PX) * 4;
                        //block copy
                        output[oOffset + 0] = tile[tileOffset + 0];
                        output[oOffset + 1] = tile[tileOffset + 1];
                        output[oOffset + 2] = tile[tileOffset + 2];

                        uint32_t alphaShift = ((PX & 3) * 4 + (PY & 3)) << 2;
                        uint8_t A = (uint8_t)((alphaBlock >> alphaShift) & 0xF);
                        output[oOffset + 3] = (uint8_t)((A << 4) | A);
                        tileOffset += 4;
                    }
                }

            }
        }
    }
    return output;
}

std::vector<uint8_t> Texture::PicaDecodeBuffer(std::vector<uint8_t> input, uint32_t width, uint32_t height, TextureFormat format) {
    if ((format == TextureFormat::ETC1) || (format == TextureFormat::ETC1A4)) {
        return ETC1Decompress(input, width, height, format == TextureFormat::ETC1A4);
    }

    uint32_t increment = BPP[format] / 8;
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

    uint32_t increment = BPP[format] / 8;
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
