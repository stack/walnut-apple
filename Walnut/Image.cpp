//
//  Image.cpp
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#include "Image.h"

#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Walnut {

    namespace Utils {
    
        static uint32_t BytesPerPixel(ImageFormat format) {
            switch (format) {
                case ImageFormat::RGBA:
                    return 4;
                    break;
                case ImageFormat::RGBA32F:
                    return 16;
                    break;
                case ImageFormat::None:
                    return 0;
                    break;
            }
            
            return 0;
        }
    
        static MTL::PixelFormat WalnutFormatToMetalFormat(ImageFormat format) {
            switch (format) {
                case ImageFormat::RGBA:
                    return MTL::PixelFormatRGBA8Unorm;
                    break;
                case ImageFormat::RGBA32F:
                    return MTL::PixelFormatRGBA32Float;
                    break;
                case ImageFormat::None:
                    return MTL::PixelFormatInvalid;
                    break;
            }
            
            return MTL::PixelFormatInvalid;
        }
    }

    Image::Image(std::string_view path) :
        filePath(path)
    {
        int width, height, channels;
        uint8_t* data = nullptr;
        
        if (stbi_is_hdr(filePath.c_str())) {
            data = (uint8_t *)stbi_loadf(filePath.c_str(), &width, &height, &channels, 4);
            format = ImageFormat::RGBA32F;
        } else {
            data = (uint8_t *)stbi_load(filePath.c_str(), &width, &height, &channels, 4);
            format = ImageFormat::RGBA;
        }
        
        this->width = width;
        this->height = height;
        
        SetData(data);
    }

    Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void* data) :
        width(width), height(height), format(format)
    {
        if (data != nullptr) {
            SetData(data);
        }
    }

    Image::~Image() {
        Application::SubmitResourceFree([inTexture = texture]() {
            inTexture->release();
        });
    }

    void Image::SetData(const void *data) {
        if (texture == nullptr) {
            auto pixelFormat = Utils::WalnutFormatToMetalFormat(format);
            auto descriptor = MTL::TextureDescriptor::texture2DDescriptor(pixelFormat, width, height, false);
            
            MTL::Device* device = Application::GetDevice();
            texture = device->newTexture(descriptor);
        }
        
        auto region = MTL::Region(0, 0, width, height);
        texture->replaceRegion(region, 0, 0, data, Utils::BytesPerPixel(format) * width, Utils::BytesPerPixel(format) * width * height);
    }
}
