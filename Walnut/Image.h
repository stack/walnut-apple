//
//  Image.hpp
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#pragma once

#include <string>

#include <Metal/Metal.hpp>

namespace Walnut {

    enum class ImageFormat {
        None = 0,
        RGBA,
        RGBA32F
    };

    class Image {
        
    public:
        
        Image(std::string_view path);
        Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);
        ~Image();
        
        void SetData(const void* data);
        
        MTL::Texture* GetDescriptorSet() const { return texture; }
        
        uint32_t GetWidth() const { return width; }
        uint32_t GetHeight() const { return height; }
        
    private:
        
        uint32_t width = 0;
        uint32_t height = 0;
        
        ImageFormat format = ImageFormat::None;
        
        MTL::Texture* texture = nullptr;
        std::string filePath;
    };
}
