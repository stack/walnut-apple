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

extern bool IsApplicationRunning;

static std::vector<std::vector<std::function<void()>>> ResourceFreeQueue;

static uint32_t CurrentFrameIndex = 0;

namespace Walnut {

    Application::Application(const ApplicationSpecification& applicationSpecification) :
        specification(applicationSpecification),
        viewSize({ 0.0, 0.0 })
    {
        Init();
    }

    Application::~Application() {
        Shutdown();

        commandQueue->release();
        metalView->release();
        window->release();
        device->release();
    }

    void Application::Init() {
        autoreleasePool = NS::AutoreleasePool::alloc()->init();
    }

    void Application::Shutdown() {
        for (auto& layer : layerStack) {
            layer->OnDetach();
        }

        layerStack.clear();

        autoreleasePool->release();

        IsApplicationRunning = false;
    }

    void Application::Run() {
        NS::Application* sharedApplication = NS::Application::sharedApplication();
        sharedApplication->setDelegate(this);
        sharedApplication->run();
    }

    void Application::Close() {
        // TODO: `stop` is not implemented
        // NS::Application* sharedApplication = NS::Application::sharedApplication();
        // sharedApplication->stop();
    }

    void Application::SubmitResourceFree(std::function<void()>&& func) {
        ResourceFreeQueue[CurrentFrameIndex].emplace_back(func);
    }

    NS::Menu* Application::CreateMenuBar() {
        using NS::StringEncoding::UTF8StringEncoding;

        NS::Menu* mainMenu = NS::Menu::alloc()->init();
        NS::MenuItem *appMenuItem = NS::MenuItem::alloc()->init();
        NS::Menu *appMenu = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

        NS::String* appName = NS::String::string(specification.Name.c_str(), UTF8StringEncoding);
        NS::String* quitItemName = NS::String::string("Quit ", UTF8StringEncoding)->stringByAppendingString(appName);
        SEL quitCallback = NS::MenuItem::registerActionCallback("appQuit", [](void*,SEL,const NS::Object* sender) {
            // TODO: How to I break the run loop here properly
            auto app = NS::Application::sharedApplication();
            app->terminate(sender);
        });

        NS::MenuItem* appQuitItem = appMenu->addItem(quitItemName, quitCallback, NS::String::string("q", UTF8StringEncoding));
        appQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
        appMenuItem->setSubmenu(appMenu);

        NS::MenuItem* windowMenuItem = NS::MenuItem::alloc()->init();
        NS::Menu* windowMenu = NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));

        SEL closeWindowCallback = NS::MenuItem::registerActionCallback("windowClose", [](void*,SEL,const NS::Object* sender) {
            // TODO: Is this even needed?
            auto app = NS::Application::sharedApplication();
            app->windows()->object<NS::Window>(0)->close();
        });

        NS::MenuItem *closeWindowItem = windowMenu->addItem(NS::String::string("Close Window", UTF8StringEncoding), closeWindowCallback, NS::String::string("w", UTF8StringEncoding));
        closeWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

        windowMenuItem->setSubmenu(windowMenu);

        mainMenu->addItem(appMenuItem);
        mainMenu->addItem(windowMenuItem);

        appMenuItem->release();
        windowMenuItem->release();
        appMenu->release();
        windowMenu->release();

        return mainMenu->autorelease();
    }

    void Application::applicationWillFinishLaunching(NS::Notification* notification) {
        NS::Menu *menu = CreateMenuBar();

        NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
        app->setMainMenu(menu);
        app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
    }

    void Application::applicationDidFinishLaunching(NS::Notification* notification) {
        CGRect frame = (CGRect){ { 100.0, 100.0 }, { static_cast<CGFloat>(specification.Width), static_cast<CGFloat>(specification.Height) } };

        window = NS::Window::alloc()->init(
            frame,
            NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled | NS::WindowStyleMaskResizable,
            NS::BackingStoreBuffered,
            false
        );

        device = MTL::CreateSystemDefaultDevice();
        commandQueue = device->newCommandQueue();

        metalView = MTK::View::alloc()->init(frame, device);
        metalView->setColorPixelFormat((MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB));
        metalView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));

        metalView->setDelegate(this);

        window->setContentView(metalView);
        window->setTitle(NS::String::string(specification.Name.c_str(), NS::StringEncoding::UTF8StringEncoding));

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplMetal_Init(device);
        ImGui_ImplOSX_Init(metalView);

        window->makeKeyAndOrderFront(nullptr);

        NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
        app->activateIgnoringOtherApps(true);
    }

    bool Application::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) {
        return true;
    }

    void Application::drawableSizeWillChange(MTK::View* view, CGSize size) {
        viewSize = size;
    }

    void Application::drawInMTKView(MTK::View* view) {
        NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
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
