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
        Material& pinkSphere = scene.materials.emplace_back();
        pinkSphere.albedo = { 1.0f, 0.0f, 1.0f };
        pinkSphere.roughness = 0.0f;
        
        Material& blueSphere = scene.materials.emplace_back();
        blueSphere.albedo = { 0.2f, 0.3f, 1.0f };
        blueSphere.roughness = 0.1f;
        
        {
            Sphere sphere;
            sphere.position = { 0.0f, 0.0f, 0.0f };
            sphere.radius = 1.0f;
            sphere.materialIndex = 0;
            
            scene.spheres.push_back(sphere);
        }
        
        {
            Sphere sphere;
            sphere.position = { 0.0f, -101.0f, -0.0f };
            sphere.radius = 100.0f;
            sphere.materialIndex = 1;
            
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
        
        ImGui::DragFloat3("Light Direction", glm::value_ptr(renderer.lightDirection), 0.1f);
        
        ImGui::End();
        
        ImGui::Begin("Scene");
        
        for (size_t i = 0; i < scene.spheres.size(); i++) {
            ImGui::PushID(static_cast<int>(i));
            
            Sphere& sphere = scene.spheres[i];
            
            ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
            ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
            ImGui::DragInt("Material", &sphere.materialIndex, 1.0f, 0, static_cast<int>(scene.materials.size() - 1));
            
            ImGui::Separator();
            
            ImGui::PopID();
        }
        
        for (size_t i = 0; i < scene.materials.size(); i++) {
            ImGui::PushID(static_cast<int>(i));
            
            Material& material = scene.materials[i];
            
            ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
            ImGui::DragFloat("Roughness", &material.roughness, 0.05f, 0.0f, 1.0f);
            ImGui::DragFloat("Metallic", &material.metallic, 0.05f, 0.0f, 1.0f);
            
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
    spec.Namespace = "us.gerstacker.ray-tracing";

    Walnut::Application* app = new Walnut::Application(spec);
    app->PushLayer<ExampleLayer>();

    // TODO: Figure out the difference in the menu bar items
    // app->SetMenubarCallback(…);

    return app;
}
