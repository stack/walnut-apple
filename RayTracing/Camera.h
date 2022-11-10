//
//  Camera.hpp
//  RayTracing
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera {
    
public:
    
    Camera(float verticalFOV, float nearClip, float farClip);
    
    bool OnUpdate(float ts);
    void OnResize(uint32_t width, uint32_t height);
    
    const glm::mat4& GetProjection() const { return projection; }
    const glm::mat4& GetInverseProjection() const { return inverseProjection; }
    const glm::mat4& GetView() const { return view; }
    const glm::mat4& GetInverseView() const { return inverseView; }
    
    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetDirection() const { return forwardDirection; }
    
    const std::vector<glm::vec3>& GetRayDirections() const { return rayDirections; }
    
    float GetRotationSpeed();
    
private:
    
    void RecalculateProjection();
    void RecalculateView();
    void RecalculateRayDirections();
    
private:
    
    glm::mat4 projection{ 1.0f };
    glm::mat4 view{ 1.0f };
    glm::mat4 inverseProjection{ 1.0f };
    glm::mat4 inverseView { 1.0f };
    
    float verticalFOV = 45.0f;
    float nearClip = 0.1f;
    float farClip = 100.0f;
    
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 forwardDirection{ 0.0f, 0.0f, 0.0f };
    
    std::vector<glm::vec3> rayDirections;
    
    glm::vec2 lastMousePosition{ 0.0f, 0.0f };
    
    uint32_t viewportWidth = 0, viewportHeight = 0;
};
