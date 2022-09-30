//
//  WalnutApp.cpp
//  WalnutApp
//
//  Created by Stephen H. Gerstacker on 2022-06-14.
//

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
    spec.Namespace = "us.gerstacker.walnut.example";

    Walnut::Application* app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();

    // TODO: Figure out the difference in the menu bar items
    // app->SetMenubarCallback(…);

    return app;
}
