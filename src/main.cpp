
#include <memory>

#define STB_IMAGE_IMPLEMENTATION

#include "logic/io/stb_image.h"
#include "../../lib/glad/glad_glfw.h"
#include "logic/opengl/OpenGLInstance.h"
#include "logic/io/IOUtils.h"
#include "logic/generator/ModelGenerator.h"
#include "presentation/gui/GUI.h"
#include "logic/state/AppState.h"
#include "presentation/scene/nodes/camera/Camera.h"
#include "presentation/scene/nodes/light/Light.h"
#include "logic/input/InputHandler.h"
#include "presentation/scene/nodes/frameBuffer/FrameBuffer.h"
#include "presentation/modules/renderers/PhongRenderModule.h"
#include "presentation/modules/renderers/RandomRenderModule.h"
#include "presentation/modules/renderers/NormalRenderModule.h"
#include "presentation/modules/gui/WorkspaceModule.h"
#include "presentation/modules/gui/MenuBarModule.h"
#include "presentation/modules/gui/LogPanelModule.h"
#include "presentation/modules/gui/DebugOverlayModule.h"
#include "presentation/modules/gui/GizmoModule.h"

// settings
const int SCR_WIDTH = 1920;
const int SCR_HEIGHT = 1080;
const int GUI_PANEL_LEFT_WIDTH = 300;

std::unique_ptr<AppState> initializeAppState() {
    const auto &viewport = Rect<int>(GUI_PANEL_LEFT_WIDTH, 0, SCR_WIDTH - GUI_PANEL_LEFT_WIDTH, SCR_HEIGHT);

    auto appState = std::make_unique<AppState>(viewport, GUI_PANEL_LEFT_WIDTH);

    Camera& camera = appState->cameras.emplace_back("Camera", SCR_WIDTH - GUI_PANEL_LEFT_WIDTH, SCR_HEIGHT, CameraMode::ANCHOR, glm::vec3(0.0f, 0.0f, 3.0f));
    appState->currentCamera = camera;
    appState->lights.emplace_back("Point Light", glm::vec3(1.0, 1.0f, 1.0f));
    auto &material = appState->materials.emplace_back(Material("Default Material", glm::vec4(1,0.5,0.5, 1), {}, 128));
    auto &mesh = appState->transformTree.meshes.emplace_back(std::make_unique<Mesh>(ModelGenerator::generateTorusMesh(50, 50)));
    mesh->material = material;

    return appState;
}

int main() {
    OpenGLInstance openGlInstance{};
    openGlInstance.init(SCR_WIDTH, SCR_HEIGHT);

    auto appState = initializeAppState();

    InputHandler inputHandler(appState);
    inputHandler.setupCallbacks(openGlInstance.getWindow());

    {
        GUI gui(openGlInstance.getWindow(), *appState);

        AssetImporter assetImporter;
        auto model = assetImporter.importModel(IOUtils::getResource("models/spitfire_mini/model/model.gltf"));

        while (openGlInstance.isRunning()) {
            OpenGLInstance::pollEvents();
            gui.newFrame();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            appState->runModules();

            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            ImGui::ShowDemoWindow();
            gui.render();

            openGlInstance.swapBuffers();
        }
    }

    glfwTerminate();
    return 0;
}
