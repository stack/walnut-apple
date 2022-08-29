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

void Renderer::Render(const Camera& camera) {
    Ray ray;
    ray.origin = camera.GetPosition();
    
    for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
            ray.direction = camera.GetRayDirections()[x + y * finalImage->GetWidth()];

            glm::vec4 color = TraceRay(ray);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            
            imageData[(y * finalImage->GetWidth()) + x] = Utils::ConvertToRGBA(color);
        }
    }

    finalImage->SetData(imageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray) {
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
    float a = glm::dot(ray.direction, ray.direction);

    // float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
    float b = 2.0f * glm::dot(ray.origin, ray.direction);

    // float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y * rayOrigin.z * rayOrigin.z - radius * radius;
    float c = glm::dot(ray.origin, ray.origin) - radius * radius;

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
    
    glm::vec3 hitPoint = ray.origin + ray.direction * closestT;
    glm::vec3 normal = glm::normalize(hitPoint - sphereOrigin);
    
    glm::vec3 lightDirection(this->lightDirection[0], this->lightDirection[1], this->lightDirection[2]);
    lightDirection = glm::normalize(lightDirection);
    
    float dot = glm::dot(normal, -lightDirection); // == cos(angle) because both parameters are unit vectors
    float intensity = glm::max(dot, 0.0f);
    
    glm::vec3 sphereColor(this->sphereColor[0], this->sphereColor[1], this->sphereColor[2]);
    sphereColor *= intensity;
    
    return glm::vec4(sphereColor, 1.0f); // RGBA
}
