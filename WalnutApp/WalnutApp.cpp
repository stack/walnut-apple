//
//  WalnutApp.cpp
//  WalnutApp
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>

#include <imgui.h>

class ExampleLayer : public Walnut::Layer
{
public:

    virtual void OnUIRender() override {
        ImGui::Begin("Hello");
        ImGui::Button("Button");
        ImGui::End();

        ImGui::ShowDemoWindow();
    }
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
    Walnut::ApplicationSpecification spec;
    spec.Name = "Walnut Example";

    Walnut::Application* app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();

    // TODO: Figure out the difference in the menu bar items
    // app->SetMenubarCallback(…);

    return app;
}
