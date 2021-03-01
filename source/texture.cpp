#include "texture.hpp"
#include "utils.hpp"
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <experimental/filesystem>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

Texture* Texture::FromBinary(TextureType inType, std::vector<uint8_t> inRawData, std::string inName, int inWidth, int inHeight) {
    Texture* tex = new Texture();

    tex->width = inWidth;
    tex->height = inHeight;
    tex->type = inType;
    tex->name = inName;
    tex->rawData = inRawData;

    tex->PrepareBitmap();

    return tex;
}

void Texture::PrepareBitmap(void) {
    bmpRGB = new uint8_t[width * height * 3];
    bmpRGBA = new uint8_t[width * height * 4];

    switch (type) {
        case TextureType::Grayscale4bpp: PrepareBitmapGrayScale4(); break;
        case TextureType::Grayscale8bpp: PrepareBitmapGrayScale8(); break;
        default:
            std::cerr << "Texture type not yet implemented" << std::endl;
    }
}

void Texture::PrepareBitmapGrayScale4(void) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x += 2) {
            for (int i = 0; i < 2; i++) {
                int pos = ((y * width) + x) / 2;
                uint8_t grayscale = 0;

                if (i == 0)
                    grayscale = (uint8_t)(rawData[pos] & 0xF0);
                else
                    grayscale = (uint8_t)((rawData[pos] & 0x0F) << 4);

                bmpRGB[(((y * width) + x + i) * 3) + 0] = grayscale;
                bmpRGB[(((y * width) + x + i) * 3) + 1] = grayscale;
                bmpRGB[(((y * width) + x + i) * 3) + 2] = grayscale;
            }
        }
    }
}

void Texture::PrepareBitmapGrayScale8(void) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pos = ((y * width) + x) * 1;

            bmpRGB[(((y * width) + x) * 3) + 0] = rawData[pos];
            bmpRGB[(((y * width) + x) * 3) + 1] = rawData[pos];
            bmpRGB[(((y * width) + x) * 3) + 2] = rawData[pos];
        }
    }
}

void Texture::Save(const path outDir, const std::string name) {
    path outfilePath = outDir / name;
    std::experimental::filesystem::create_directories(outfilePath.parent_path());

    switch (type) {
        case TextureType::Grayscale4bpp:
            outfilePath += ".i4.png";
            stbi_write_png(outfilePath.c_str(), width, height, 3, bmpRGB, width * 3);
            break;
        case TextureType::Grayscale8bpp:
            outfilePath += ".i8.png";
            stbi_write_png(outfilePath.c_str(), width, height, 3, bmpRGB, width * 3);
            break;
        default:
            std::cerr << "Texture type not yet implemented" << std::endl;
    }
}
