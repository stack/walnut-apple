//
//  Layer.h
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

#pragma once

namespace Walnut {

    class Layer {

    public:

        virtual ~Layer() = default;

        virtual void OnAttach() {};
        virtual void OnDetach() {};

        virtual void OnUIRender() {};
    };
}
