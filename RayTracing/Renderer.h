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
    
    struct Settings {
        bool accumulate = true;
    };
    
public:

    Renderer() = default;

    void OnResize(uint32_t width, uint32_t height);
    void Render(const Scene& scene, const Camera& camera);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return finalImage; }
    
    void ResetFrameIndex() { frameIndex = 1; }
    
    Settings& GetSettings() { return settings; }
    
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
    Settings settings;
    
    std::vector<uint32_t> imageHorizontalIterator;
    std::vector<uint32_t> imageVerticalIterator;
    
    glm::vec4* accumulationData = nullptr;
    
    uint32_t frameIndex = 1;
};
