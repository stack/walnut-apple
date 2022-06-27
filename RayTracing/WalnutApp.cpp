//
//  WalnutApp.cpp
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Timer.h>

#include <imgui.h>

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:

    virtual void OnUIRender() override {
        ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", lastRenderTime);
        
        if (ImGui::Button("Render")) {
            Render();
        }
        
        ImGui::End();
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        
        viewportWidth = ImGui::GetContentRegionAvail().x;
        viewportHeight = ImGui::GetContentRegionAvail().y;

        auto image = renderer.GetFinalImage();

        if (image != nullptr) {
            ImGui::Image(
                image->GetDescriptorSet(),
                { (float)image->GetWidth(), (float)image->GetHeight() },
                ImVec2(0, 1),
                ImVec2(1, 0)
            );
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
        
        Render();
    }
    
    void Render() {
        Timer timer;

        renderer.OnResize(viewportWidth, viewportHeight);
        renderer.Render();
        
        lastRenderTime = timer.ElapsedMillis();
    }
    
private:
    Renderer renderer;
    uint32_t viewportWidth = 0, viewportHeight = 0;
    
    float lastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
    Walnut::ApplicationSpecification spec;
    spec.Name = "Ray Tracing";

    Walnut::Application* app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();

    // TODO: Figure out the difference in the menu bar items
    // app->SetMenubarCallback(…);

    return app;
}
