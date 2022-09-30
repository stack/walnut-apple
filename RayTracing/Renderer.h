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
#include "Scene.h"

#include <memory>

class Renderer {

public:

    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render(const Scene& scene, const Camera& camera);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
    
public:
    
    glm::vec3 lightDirection { -1.0f, -1.0f, -1.0f };

private:
    
    struct HitPayload {
        float hitDistance;
        glm::vec3 worldPosition;
        glm::vec3 worldNormal;
        
        int objectIndex;
    };

    glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen
    
    HitPayload TraceRay(const Ray& ray);
    HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
    HitPayload Miss(const Ray& ray);

private:
    const Scene* activeScene = nullptr;
    const Camera* activeCamera = nullptr;

    std::shared_ptr<Walnut::Image> finalImage;
    uint32_t* imageData = nullptr;

};
