//
//  Renderer.h
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-27.
//

#pragma once

#include <Walnut/Image.h>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Ray.h"

#include <memory>

class Renderer {

public:

    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render(const Camera& camera);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
    
public:
    
    float sphereColor[3] = { 1.0f, 0.0f, 1.0f };
    float lightDirection[3] = { -1.0f, -1.0f, -1.0f } ;

private:

    glm::vec4 TraceRay(const Ray& ray);

private:

    std::shared_ptr<Walnut::Image> finalImage;
    uint32_t* imageData = nullptr;

};
