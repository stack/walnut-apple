//
//  Scene.h
//  Walnut
//
//  Created by Stephen Gerstacker on 2022-09-26.
//

#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Material {
    glm::vec3 albedo { 1.0f };
    float roughness = 1.0f;
    float metallic = 0.0f;
};

struct Sphere {
    glm::vec3 position { 0.0f, 0.0f, 0.0f };
    float radius = 0.5f;
    
    int materialIndex = 0;
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Material> materials;
};
