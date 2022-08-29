//
//  Camera.cpp
//  RayTracing
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip, float farClip) :
    verticalFOV(verticalFOV),
    nearClip(nearClip),
    farClip(farClip)
{
    forwardDirection = glm::vec3(0, 0, -1);
    position = glm::vec3(0, 0, 3);
}

void Camera::OnUpdate(float ts) {
    glm::vec2 mousePosition = Input::GetMousePosition();
    glm::vec2 delta = (mousePosition - lastMousePosition) * 0.002f;
    lastMousePosition = mousePosition;
    
    if (!Input::IsMouseButtonDown(MouseButton::Right)) {
        Input::SetCursorMode(CursorMode::Normal);
        return;
    }
    
    Input::SetCursorMode(CursorMode::Locked);
    
    bool moved = false;
    
    constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
    glm::vec3 rightDirection = glm::cross(forwardDirection, upDirection);
    
    float speed = 5.0f;
    
    // Movement
    if (Input::IsKeyDown(KeyCode::W)) {
        position += forwardDirection * speed * ts;
        moved = true;
    } else if (Input::IsKeyDown(KeyCode::S)) {
        position -= forwardDirection * speed * ts;
        moved = true;
    }
    
    if (Input::IsKeyDown(KeyCode::A)) {
        position -= rightDirection * speed * ts;
        moved = true;
    } else if (Input::IsKeyDown(KeyCode::D)) {
        position += rightDirection * speed * ts;
        moved = true;
    }
    
    if (Input::IsKeyDown(KeyCode::Q)) {
        position -= upDirection * speed * ts;
        moved = true;
    } else if (Input::IsKeyDown(KeyCode::E)) {
        position += upDirection * speed * ts;
        moved = true;
    }
    
    // Rotation
    if (delta.x != 0.0f || delta.y != 0.0f) {
        float pitchDelta = delta.y * GetRotationSpeed();
        float yawDelta = delta.x * GetRotationSpeed();
        
        glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection), glm::angleAxis(-yawDelta, glm::vec3(0.0f, 1.0f, 0.0f))));
        forwardDirection = glm::rotate(q, forwardDirection);
        
        moved = true;
    }
    
    if (moved) {
        RecalculateView();
        RecalculateRayDirections();
    }
}

void Camera::OnResize(uint32_t width, uint32_t height) {
    if (width == viewportWidth && height == viewportHeight) {
        return;
    }
    
    viewportWidth = width;
    viewportHeight = height;
    
    RecalculateProjection();
    RecalculateRayDirections();
}

float Camera::GetRotationSpeed() {
    return 0.3f;
}

void Camera::RecalculateProjection() {
    projection = glm::perspectiveFov(glm::radians(verticalFOV), (float)viewportWidth, (float)viewportHeight, nearClip, farClip);
    inverseProjection = glm::inverse(projection);
}

void Camera::RecalculateView() {
    view = glm::lookAt(position, position + forwardDirection, glm::vec3(0, 1, 0));
    inverseView = glm::inverse(view);
}

void Camera::RecalculateRayDirections() {
    rayDirections.resize(viewportWidth * viewportHeight);
    
    for (uint32_t y = 0; y < viewportHeight; y++) {
        for (uint32_t x = 0; x < viewportWidth; x++) {
            glm::vec2 coord = { (float)x / (float)viewportWidth, (float)y / (float)viewportHeight };
            coord = coord * 2.0f - 1.0f; // -1 -> 1
            
            glm::vec4 target = inverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
            glm::vec3 rayDirection = glm::vec3(inverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
            rayDirections[x + y * viewportWidth] = rayDirection;
        }
    }
}
