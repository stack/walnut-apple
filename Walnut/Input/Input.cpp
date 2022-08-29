//
//  Input.cpp
//  Walnut
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#include "Input.h"

#include "Application.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Walnut {

    bool Input::IsKeyDown(KeyCode keycode) {
        GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
        int state = glfwGetKey(windowHandle, (int)keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonDown(MouseButton button) {
        GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
        int state = glfwGetMouseButton(windowHandle, (int)button);
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition() {
        GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
        
        double x, y;
        glfwGetCursorPos(windowHandle, &x, &y);
        
        return { (float)x, (float)y };
    }

    void Input::SetCursorMode(CursorMode mode) {
        GLFWwindow* windowHandle = Application::Get().GetWindowHandle();
        glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
    }
}
