//
//  WalnutApp.cpp
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Image.h>
#include <Walnut/Random.h>
#include <Walnut/Timer.h>

#include <imgui.h>

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
        
        if (image != nullptr) {
            ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() } );
        }
        
        ImGui::End();
        ImGui::PopStyleVar();
        
        Render();
    }
    
    void Render() {
        Timer timer;
        
        if (image == nullptr || viewportWidth != image->GetWidth() || viewportHeight != image->GetHeight()) {
            image = std::make_shared<Image>(viewportWidth, viewportHeight, ImageFormat::RGBA);
            delete[] imageData;
            imageData = new uint32_t[viewportWidth * viewportHeight];
        }
        
        for (uint32_t i = 0; i < viewportWidth * viewportHeight; i++) {
            imageData[i] = Random::UInt();
            imageData[i] |= 0xff000000;
        }
        
        image->SetData(imageData);
        
        lastRenderTime = timer.ElapsedMillis();
    }
    
private:
    std::shared_ptr<Image> image;
    uint32_t* imageData = nullptr;
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
