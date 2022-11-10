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
    } else {
        if (finalImage->GetWidth() == width && finalImage->GetHeight() == height) {
            return;
        }
        
        finalImage->Resize(width, height);
    }
    
    delete[] imageData;
    imageData = new uint32_t[width * height];
    
    delete[] accumulationData;
    accumulationData = new glm::vec4[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera) {
    activeScene = &scene;
    activeCamera = &camera;
    
    if (frameIndex == 1) {
        memset(accumulationData, 0, finalImage->GetWidth() * finalImage->GetHeight() * sizeof(glm::vec4));
    }
    
    for (uint32_t y = 0; y < finalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < finalImage->GetWidth(); x++) {
            glm::vec4 color = PerPixel(x, y);
            accumulationData[(y * finalImage->GetWidth()) + x] += color;
            
            glm::vec4 accumulatedColor = accumulationData[(y * finalImage->GetWidth()) + x];
            accumulatedColor /= static_cast<float>(frameIndex);
            
            accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));

            imageData[(y * finalImage->GetWidth()) + x] = Utils::ConvertToRGBA(accumulatedColor);
        }
    }

    finalImage->SetData(imageData);
    
    if (settings.accumulate) {
        frameIndex += 1;
    } else {
        frameIndex = 1;
    }
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y) {
    Ray ray;
    ray.origin = activeCamera->GetPosition();
    ray.direction = activeCamera->GetRayDirections()[x + y * finalImage->GetWidth()];
    
    int bounces = 5;
    glm::vec3 color(0.0f);
    float multiplier = 1.0f;
    
    for (int bounce = 0; bounce < bounces; bounce++) {
        Renderer::HitPayload payload = TraceRay(ray);
        
        if (payload.hitDistance < 0.0f) {
            glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
            color += skyColor * multiplier;
            
            break;
        }
        
        glm::vec3 lightDirection = glm::normalize(this->lightDirection);
        
        float dot = glm::dot(payload.worldNormal, -lightDirection); // == cos(angle) because both parameters are unit vectors
        float intensity = glm::max(dot, 0.0f);
        
        const Sphere& closestSphere = activeScene->spheres[payload.objectIndex];
        const Material& material = activeScene->materials[closestSphere.materialIndex];
        
        glm::vec3 sphereColor = material.albedo;
        sphereColor *= intensity;
        
        color += sphereColor * multiplier;
        
        multiplier *= 0.5f;
        
        ray.origin = payload.worldPosition + payload.worldNormal * 0.0001f;
        ray.direction = glm::reflect(ray.direction, payload.worldNormal + material.roughness
                                      * Walnut::Random::Vec3(-0.5f, 0.5f));
    }
    
    return glm::vec4(color, 1.0f); // RGBA
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) {
    Renderer::HitPayload payload;
    payload.hitDistance = hitDistance;
    payload.objectIndex = objectIndex;
    
    const Sphere& closestSphere = activeScene->spheres[objectIndex];
    
    glm::vec3 origin = ray.origin - closestSphere.position;
    payload.worldPosition = origin + ray.direction * hitDistance;
    payload.worldNormal = glm::normalize(payload.worldPosition);
    payload.worldPosition += closestSphere.position;
    
    return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray) {
    Renderer::HitPayload payload;
    payload.hitDistance = -1.0f;
    
    return payload;
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) {
    int closestSphere = -1;
    float hitDistance = std::numeric_limits<float>::max();
    
    for (size_t sphereIndex = 0; sphereIndex < activeScene->spheres.size(); sphereIndex++) {
        const Sphere& sphere = activeScene->spheres[sphereIndex];
        
        // (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0]
        // where
        // a = ray origin
        // b = ray direction
        // r = radius
        // t = hit distance

        // NOTE: a, b, & c are the quadratic variables, not the ones mentioned above. These are the coefficients above

        glm::vec3 origin = ray.origin - sphere.position;
        
        // float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
        float a = glm::dot(ray.direction, ray.direction);

        // float b = 2.0f * (rayOrigin.x * rayDirection.x + rayOrigin.y * rayDirection.y + rayOrigin.z * rayDirection.z);
        float b = 2.0f * glm::dot(origin, ray.direction);

        // float c = rayOrigin.x * rayOrigin.x + rayOrigin.y * rayOrigin.y * rayOrigin.z * rayOrigin.z - radius * radius;
        float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

        // Quadratic formula discriminant
        // b^2 - 4ac
        float discriminant = b * b - 4.0f * a * c;

        // No hit, so go to the next sphere
        if (discriminant < 0.0f) {
            continue;
        }
        
        // Full quadratic equation
        // (-b +- sqrt(discriminant)) / (2.0f * a)
        // float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
        // float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
        float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
        
        if (closestT > 0.0f && closestT < hitDistance) {
            closestSphere = static_cast<uint32_t>(sphereIndex);
            hitDistance = closestT;
        }
    }
    
    if (closestSphere == -1) {
        return Miss(ray);
    } else {
        return ClosestHit(ray, hitDistance, closestSphere);
    }
}
