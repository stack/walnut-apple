//
//  Application.h
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

#pragma once

#include "Layer.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

namespace Walnut {

    struct ApplicationSpecification {
        std::string Name = "Walnut App";
        uint32_t Width = 1600;
        uint32_t Height = 900;
    };

    class Application : public NS::ApplicationDelegate, public MTK::ViewDelegate {

    public:
        // Application Methods
        Application(const ApplicationSpecification& applicationSpecification);
        ~Application();
        
        static MTL::Device* GetDevice();

        void Run();
        void SetMenubarCallback(const std::function<void()>& callback) { menubarCallback = callback; }

        template<typename T>
        void PushLayer() {
            static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
            layerStack.emplace_back(std::make_shared<T>())->OnAttach();
        }

        void PushLayer(const std::shared_ptr<Layer>& layer) { layerStack.emplace_back(layer); layer->OnAttach(); }

        void Close();

        static void SubmitResourceFree(std::function<void()>&& func);

        // ApplicationDelegate Protocol
        virtual void applicationWillFinishLaunching(NS::Notification* notification) override;
        virtual void applicationDidFinishLaunching(NS::Notification* notification) override;
        virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override;

        // MTKViewDelegate Protocol
        virtual void drawableSizeWillChange(MTK::View* view, CGSize size) override;
        virtual void drawInMTKView(MTK::View* view) override;

    private:
        // Application Methods
        void Init();
        void Shutdown();

        // ApplicationDelegate Protocol
        NS::Menu* CreateMenuBar();

    private:

        ApplicationSpecification specification;

        NS::AutoreleasePool* autoreleasePool;

        std::vector<std::shared_ptr<Layer>> layerStack;
        std::function<void()> menubarCallback;

        NS::Window* window;
        MTK::View* metalView;
        CGSize viewSize;
        
        dispatch_semaphore_t commandSemaphore;
        MTL::CommandQueue* commandQueue;
    };

    Application* CreateApplication(int argc, char** argv);
}
