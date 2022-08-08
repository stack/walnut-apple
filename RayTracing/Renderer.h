//
//  Renderer.h
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-27.
//

#pragma once

#include <Walnut/Image.h>

#include <glm/glm.hpp>

#include <memory>

class Renderer {

public:

    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render();

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }

private:

    glm::vec4 PerPixel(glm::vec2 coord);

private:

    std::shared_ptr<Walnut::Image> finalImage;
    uint32_t* imageData = nullptr;

};
