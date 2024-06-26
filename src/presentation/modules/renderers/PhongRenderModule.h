//
// Created by faliszewskii on 12.02.24.
//

#ifndef OPENGL_SANDBOX_PHONGRENDERMODULE_H
#define OPENGL_SANDBOX_PHONGRENDERMODULE_H


#include "../Module.h"
#include "../../../logic/algebra/Rect.h"
#include "../../../logic/io/IOUtils.h"
#include "RenderHelpers.h"

class PhongRenderModule : public Module {
    const int workspaceWidth;
    Shader shader;

public:
    explicit PhongRenderModule(int workspaceWidth, bool active) : Module(active), workspaceWidth(workspaceWidth),
                                                     shader(Shader("blinnPhong", IOUtils::getResource("shaders/phong/basicBlinnPhong.vert"), IOUtils::getResource("shaders/phong/basicBlinnPhong.frag"))) {}

    void run(AppState &appState) final {
        ImGuiIO &io = ImGui::GetIO();
        glViewport(workspaceWidth, 0, io.DisplaySize.x - workspaceWidth, io.DisplaySize.y);
        shader.use();

        if(!appState.currentCamera) return;
        Camera &camera = appState.currentCamera.value().get();
        RenderHelpers::setUpCamera(camera, {shader});
        RenderHelpers::setUpLights(appState.lights, {shader});
        RenderHelpers::renderMeshes(appState.transformTree, {shader}, overloaded{
                [&](std::unique_ptr<Mesh<Vertex>>& mesh) {
                    RenderHelpers::setUpMesh(*mesh, {shader});
                    mesh->render();
                },
                [&](std::unique_ptr<MeshGenerator>& generator) {
                    auto &mesh = generator->getTargetMesh();
                    RenderHelpers::setUpMesh(mesh, {shader});
                    mesh.render();
                },
                [&](std::unique_ptr<Point>& point) {
                    auto &mesh = *point->mesh;
                    RenderHelpers::setUpMesh(mesh, {shader});
                    // TODO Should points have position as well? For maths f.e. Maybe not... for consistency. well, they do have it now.
                    mesh.render();
                }
        });
    };

    [[nodiscard]] std::string getName() const final {
        return "Phong Render Module";
    }
};


#endif //OPENGL_SANDBOX_PHONGRENDERMODULE_H
