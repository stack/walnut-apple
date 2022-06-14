//
//  Application.mm
//  Walnut
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

#include "Application.h"


extern bool IsApplicationRunning;

static std::vector<std::vector<std::function<void()>>> ResourceFreeQueue;

static uint32_t CurrentFrameIndex = 0;

namespace Walnut {

    Application::Application(const ApplicationSpecification& applicationSpecification) :
        specification(applicationSpecification)
    {
        Init();
    }

    Application::~Application() {
        Shutdown();
    }

    void Application::Init() {
        autoreleasePool = NS::AutoreleasePool::alloc()->init();
        appDelegate = new ApplicationDelegate(specification);
    }

    void Application::Shutdown() {
        for (auto& layer : layerStack) {
            layer->OnDetach();
        }

        layerStack.clear();

        delete appDelegate;

        autoreleasePool->release();

        IsApplicationRunning = false;
    }

    void Application::Run() {
        NS::Application* sharedApplication = NS::Application::sharedApplication();
        sharedApplication->setDelegate(appDelegate);
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

    ApplicationDelegate::ApplicationDelegate(const ApplicationSpecification &applicationSpecification) :
        specification(applicationSpecification)
    {

    }

    ApplicationDelegate::~ApplicationDelegate() {
        metalView->release();
        window->release();
        metalDevice->release();
        delete viewDelegate;
    }

    NS::Menu* ApplicationDelegate::CreateMenuBar() {
        using NS::StringEncoding::UTF8StringEncoding;

        NS::Menu* mainMenu = NS::Menu::alloc()->init();
        NS::MenuItem *appMenuItem = NS::MenuItem::alloc()->init();
        NS::Menu *appMenu = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

        NS::String* appName = NS::RunningApplication::currentApplication()->localizedName();
        NS::String* quitItemName = NS::String::string("Quit", UTF8StringEncoding)->stringByAppendingString(appName);
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

    void ApplicationDelegate::applicationWillFinishLaunching(NS::Notification* notification) {
        NS::Menu *menu = CreateMenuBar();

        NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
        app->setMainMenu(menu);
        app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
    }

    void ApplicationDelegate::applicationDidFinishLaunching(NS::Notification* notification) {
        // TODO: Read this from the spec
        CGRect frame = (CGRect){ { 100.0, 100.0 }, { static_cast<CGFloat>(specification.Width), static_cast<CGFloat>(specification.Height) } };

        window = NS::Window::alloc()->init(
            frame,
            NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
            NS::BackingStoreBuffered,
            false
        );

        metalDevice = MTL::CreateSystemDefaultDevice();

        metalView = MTK::View::alloc()->init(frame, metalDevice);
        metalView->setColorPixelFormat((MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB));
        metalView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));

        viewDelegate = new Walnut::ViewDelegate(metalDevice);
        metalView->setDelegate(viewDelegate);

        // TODO: Read this from the spec
        window->setContentView(metalView);
        window->setTitle(NS::String::string(specification.Name.c_str(), NS::StringEncoding::UTF8StringEncoding));

        window->makeKeyAndOrderFront(nullptr);

        NS::Application* app = reinterpret_cast<NS::Application*>(notification->object());
        app->activateIgnoringOtherApps(true);
    }

    bool ApplicationDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) {
        return true;
    }

    ViewDelegate::ViewDelegate(MTL::Device* device) :
        MTK::ViewDelegate(),
        device(device->retain())
    {
        commandQueue = device->newCommandQueue();
    }

    ViewDelegate::~ViewDelegate() {
        commandQueue->release();
        device->release();
    }

    void ViewDelegate::drawInMTKView(MTK::View* view) {
        NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();

        MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
        MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

        encoder->endEncoding();
        commandBuffer->presentDrawable(view->currentDrawable());
        commandBuffer->commit();

        autoreleasePool->release();
    }
}
