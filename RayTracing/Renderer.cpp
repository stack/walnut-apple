//
//  Renderer.cpp
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-27.
//

#include "Renderer.h"

#include <Walnut/Random.h>

void Renderer::OnResize(uint32_t width, uint32_t height) {
    if (finalImage == nullptr) {
        finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
        imageData = new uint32_t[width * height];
    } else {
        if (finalImage->GetWidth() != width || finalImage->GetHeight() != height) {
            finalImage->Resize(width, height);

            delete[] imageData;
            imageData = new uint32_t[width * height];
        }
    }
}

void Renderer::Render() {
    for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
            glm::vec2 coord = { (float)x / (float)finalImage->GetWidth(), (float)y / (float)finalImage->GetHeight() };
            coord = coord * 2.0f - 1.0f; // Normalize from 0 -> 1 to -1 -> 1

            imageData[(y * finalImage->GetWidth()) + x] = PerPixel(coord);
        }
    }

    finalImage->SetData(imageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord) {
    glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
    glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
    // rayDirection = glm::normalize(rayDirection);

    float radius = 0.5;

    // (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0]
    // where
    // a = ray origin
    // b = ray direction
    // r = radius
    // t = hit distance

    // NOTE: a, b, & c are the quadratic variables, not the ones mentioned above. These are the coefficients above

    // float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
    float a = glm::dot(rayDirection, rayDirection);

    // float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
    float b = 2.0f * glm::dot(rayOrigin, rayDirection);

    // float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y * rayOrigin.z * rayOrigin.z - radius * radius;
    float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

    // Quadratic formula discriminant
    // b^2 - 4ac
    float discriminant = b * b - 4.0f * a * c;

    if (discriminant >= 0.0f) {
        return 0xffff00ff;
    } else {
        return 0xff000000;
    }
}
