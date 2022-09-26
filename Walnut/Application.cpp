//
//  Application.mm
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

#include "Application.h"

#include <imgui.h>
#include <backends/imgui_impl_metal.h>
#include <backends/imgui_impl_osx.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#pragma clang diagnostic pop

#include <glm/glm.hpp>

#include "Utilities.h"

#include <iostream>

extern bool IsApplicationRunning;

static std::vector<std::vector<std::function<void()>>> ResourceFreeQueue;

static const int MaxFramesInFlight = 3;
static uint32_t CurrentFrameIndex = 0;
static MTL::Device* MetalDevice = nullptr;

static Walnut::Application* ApplicationInstance = nullptr;

static void GlfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %i: %s\n", error, description);
}

namespace Walnut {

    Application::Application(const ApplicationSpecification& applicationSpecification) :
        specification(applicationSpecification),
        viewSize({ 0.0, 0.0 })
    {
        ApplicationInstance = this;
        
        Init();
    }

    Application::~Application() {
        Shutdown();

        commandQueue->release();
        metalView->release();
    }

    Application& Application::Get() {
        return *ApplicationInstance;
    }

    void Application::Init() {
        // Setup GLFW window
        glfwSetErrorCallback(GlfwErrorCallback);
        
        if (!glfwInit()) {
            std::cerr << "Could not initialize GLFW!" << std::endl;
            return;
        }
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHintString(GLFW_COCOA_FRAME_NAME, "WalnutApp");
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
        windowHandle = glfwCreateWindow(specification.Width, specification.Height, specification.Name.c_str(), NULL, NULL);
        
        // Setup Metal
        CGRect frame = (CGRect){ { 0.0, 0.0 }, { static_cast<CGFloat>(specification.Width), static_cast<CGFloat>(specification.Height) } };
        
        MetalDevice = MTL::CreateSystemDefaultDevice();
        commandQueue = MetalDevice->newCommandQueue();

        metalView = MTK::View::alloc()->init(frame, MetalDevice);
        metalView->setColorPixelFormat((MTL::PixelFormat::PixelFormatBGRA8Unorm));
        metalView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));
        metalView->setPaused(true);
        metalView->setEnableSetNeedsDisplay(false);

        metalView->setDelegate(this);
        
        ResourceFreeQueue.resize(MaxFramesInFlight);
        
        NS::Window* window = reinterpret_cast<NS::Window*>(glfwGetCocoaWindow(windowHandle));
        AddViewToWindow(metalView, window);
        
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplMetal_Init(MetalDevice);
        ImGui_ImplOSX_Init(metalView);
        
        auto bundle = NS::Bundle::mainBundle();
        auto resourcePath = bundle->resourcePath();
        auto fontPath = resourcePath->stringByAppendingString(NS::String::string("/SF-Mono-Regular.otf", NS::StringEncoding::UTF8StringEncoding));
        io.Fonts->AddFontFromFileTTF(fontPath->utf8String(), 13.0f);
        
        // Setup Metal / Apple specific stuff
        autoreleasePool = NS::AutoreleasePool::alloc()->init();
        commandSemaphore = dispatch_semaphore_create(MaxFramesInFlight);
    }

    void Application::Shutdown() {
        for (auto& layer : layerStack) {
            layer->OnDetach();
        }

        layerStack.clear();
        
        for (auto& queue : ResourceFreeQueue) {
            for (auto& func : queue) {
                func();
            }
        }
        
        ResourceFreeQueue.clear();
        
        ImGui_ImplMetal_Shutdown();
        ImGui_ImplOSX_Shutdown();
        ImGui::DestroyContext();
        
        metalView->release();
        metalView = nullptr;
        
        glfwDestroyWindow(windowHandle);
        windowHandle = nullptr;
        
        glfwTerminate();

        autoreleasePool->release();

        IsApplicationRunning = false;
    }

    void Application::Run() {
        isRunning = true;
        
        // Main loop
        while (!glfwWindowShouldClose(windowHandle) && isRunning) {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();
            
            for (auto& layer : layerStack) {
                layer->OnUpdate(timeStep);
            }
            
            metalView->draw();
            
            float time = GetTime();
            frameTime = time - lastFrameTime;
            timeStep = glm::min<float>(frameTime, 0.0333f);
            lastFrameTime = time;
        }
    }

    void Application::Close() {
        isRunning = false;
    }

    float Application::GetTime() {
        return (float)glfwGetTime();
    }

    MTL::Device* Application::GetDevice() {
        return MetalDevice;
    }

    void Application::SubmitResourceFree(std::function<void()>&& func) {
        ResourceFreeQueue[CurrentFrameIndex].emplace_back(func);
    }

    void Application::drawableSizeWillChange(MTK::View* view, CGSize size) {
        viewSize = size;
    }

    void Application::drawInMTKView(MTK::View* view) {
        NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
        
        CurrentFrameIndex = (CurrentFrameIndex + 1) % MaxFramesInFlight;

        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
        
        int thisFrameIndex = CurrentFrameIndex;
        dispatch_semaphore_wait(commandSemaphore, DISPATCH_TIME_FOREVER);
        commandBuffer->addCompletedHandler(^void(MTL::CommandBuffer* commandBuffer) {
            dispatch_semaphore_signal(commandSemaphore);
            
            for (auto& func : ResourceFreeQueue[thisFrameIndex]) {
                func();
            }
            
            ResourceFreeQueue[thisFrameIndex].clear();
        });
        
        MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

        ImGui_ImplMetal_NewFrame(renderPassDescriptor);
        ImGui_ImplOSX_NewFrame(metalView);
        ImGui::NewFrame();

        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus |  ImGuiWindowFlags_NoNavFocus;

        if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode) {
            windowFlags |= ImGuiWindowFlags_NoBackground;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, windowFlags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        ImGuiIO& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspaceID = ImGui::GetID("MetalAppDockspace");
            ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
        }

        for (auto& layer : layerStack) {
            layer->OnUIRender();
        }

        ImGui::End(); // Dockspace end

        ImGui::Render();
        ImDrawData* mainDrawData = ImGui::GetDrawData();
        
        encoder->pushDebugGroup(NS::String::string("Dear ImGui rendering", NS::StringEncoding::UTF8StringEncoding));
        ImGui_ImplMetal_RenderDrawData(mainDrawData, commandBuffer, encoder);
        encoder->popDebugGroup();

        encoder->endEncoding();
        commandBuffer->presentDrawable(view->currentDrawable());
        commandBuffer->commit();

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        autoreleasePool->release();
    }
}
