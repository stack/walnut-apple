//
//  WalnutApp.cpp
//  RayTracing
//
//  Created by Stephen H. Gerstacker on 2022-06-16.
//

#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>
#include <Walnut/Timer.h>

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>

#include "Camera.h"
#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:

    ExampleLayer() :
        camera(45.0f, 0.1f, 100.0f)
    {
        {
            Sphere sphere;
            sphere.position = { 0.0f, 0.0f, 0.0f };
            sphere.radius = 0.5f;
            sphere.albedo = { 1.0f, 0.0f, 1.0f };
            
            scene.spheres.push_back(sphere);
        }
        
        {
            Sphere sphere;
            sphere.position = { 1.0f, 0.0f, -5.0f };
            sphere.radius = 1.5f;
            sphere.albedo = { 0.2f, 0.3f, 1.0f };
            
            scene.spheres.push_back(sphere);
        }
    }
    
    virtual void OnUpdate(float ts) override {
        camera.OnUpdate(ts);
    }
    
    virtual void OnUIRender() override {
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
        
        ImGui::Begin("Settings");
        ImGui::Text("Last render: %.3fms", lastRenderTime);
        ImGui::Text("Viewport: %ux%u", viewportWidth, viewportHeight);
        
        ImGui::Separator();
        
        ImGui::SliderFloat3("Light Direction", (float *)&(renderer.lightDirection), -1.0f, 1.0f, "%0.01f");
        
        ImGui::End();
        
        ImGui::Begin("Scene");
        
        for (size_t i = 0; i < scene.spheres.size(); i++) {
            ImGui::PushID(static_cast<int>(i));
            
            Sphere& sphere = scene.spheres[i];
            
            ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
            ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
            ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.albedo));
            
            ImGui::Separator();
            
            ImGui::PopID();
        }
        
        ImGui::End();
        
        Render();
    }
    
    void Render() {
        Timer timer;

        camera.OnResize(viewportWidth, viewportHeight);
        renderer.OnResize(viewportWidth, viewportHeight);
        
        renderer.Render(scene, camera);
        
        lastRenderTime = timer.ElapsedMillis();
    }
    
private:
    Camera camera;
    Renderer renderer;
    Scene scene;
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
