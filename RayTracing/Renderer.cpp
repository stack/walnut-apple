//
//  Renderer.cpp
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-27.
//

#include "Renderer.h"

#include <Walnut/Random.h>

namespace Utils {

    static uint32_t ConvertToRGBA(const glm::vec4& color) {
        uint8_t r = (uint8_t)(color.r * 255.0f);
        uint8_t g = (uint8_t)(color.g * 255.0f);
        uint8_t b = (uint8_t)(color.b * 255.0f);
        uint8_t a = (uint8_t)(color.a * 255.0f);
        
        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        
        return result;
    }
}

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
    auto width = finalImage->GetWidth();
    auto height = finalImage->GetHeight();
    float aspectRatio = (float)width / (float)height;
    
    for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
            glm::vec2 coord = { (float)x / (float)finalImage->GetWidth(), (float)y / (float)finalImage->GetHeight() };
            coord = coord * 2.0f - 1.0f; // Normalize from 0 -> 1 to -1 -> 1
            
            coord.x *= aspectRatio;

            glm::vec4 color = PerPixel(coord);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            
            imageData[(y * finalImage->GetWidth()) + x] = Utils::ConvertToRGBA(color);
        }
    }

    finalImage->SetData(imageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord) {
    glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
    glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
    // rayDirection = glm::normalize(rayDirection);
    
    glm::vec3 sphereOrigin(0.0f, 0.0f, 0.0f);
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

    // No hit, so exit early
    if (discriminant < 0.0f) {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // RGBA
    }
    
    // Full quadratic equation
    // (-b +- sqrt(discriminant)) / (2.0f * a)
    // float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
    // float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
    float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
    
    glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;
    glm::vec3 normal = glm::normalize(hitPoint - sphereOrigin);
    
    glm::vec3 lightDirection(-1.0f, -1.0f, -1.0f);
    lightDirection = glm::normalize(lightDirection);
    
    float dot = glm::dot(normal, -lightDirection); // == cos(angle) because both parameters are unit vectors
    float intensity = glm::max(dot, 0.0f);
    
    glm::vec3 sphereColor(1.0f, 0.0f, 1.0f); // RGB
    sphereColor *= intensity;
    
    return glm::vec4(sphereColor, 1.0f); // RGBA

    
    
    /* OLD TEST CODE */
    /*
    std::vector<float> values = {
        (-b - sqrt(discriminant)) / (2.0f * a),
        (-b + sqrt(discriminant)) / (2.0f * a),
    };
    
    glm::vec3 hitPosition = rayOrigin + rayDirection * values[1];
    glm::vec3 normal = glm::normalize(hitPosition - sphereOrigin);
    
    float lightIntensity = glm::max(glm::dot(normal, -lightDirection), 0.0f);
    
    glm::vec3 sphereColor(1.0f, 0.0f, 1.0f);
    sphereColor *= lightIntensity;
    sphereColor *= 255.0f;
    
    uint32_t color = 0xff000000;
    color += ((uint32_t)sphereColor.x) << 16;
    color += ((uint32_t)sphereColor.y) << 8;
    color += ((uint32_t)sphereColor.z) << 0;
    
    return color;
     */
}
