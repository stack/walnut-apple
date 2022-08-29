//
//  Input.hpp
//  Walnut
//
//  Created by Stephen Gerstacker on 2022-08-29.
//

#pragma once

#include "KeyCodes.h"

#include <glm/glm.hpp>

namespace Walnut {

    class Input {
            
    public:
        static bool IsKeyDown(KeyCode keycode);
        static bool IsMouseButtonDown(MouseButton button);
        
        static glm::vec2 GetMousePosition();
        
        static void SetCursorMode(CursorMode mode);
    };
}
