
#include <memory>

#define STB_IMAGE_IMPLEMENTATION

#include "logic/io/stb_image.h"
#include "../../lib/glad/glad_glfw.h"
#include "logic/opengl/OpenGLInstance.h"
#include "logic/io/IOUtils.h"
#include "logic/generator/ModelGenerator.h"
#include "presentation/gui/GUI.h"
#include "logic/ApplicationState.h"
#include "presentation/rendering/renderVisitor/RenderSceneVisitor.h"
#include "presentation/scene/nodes/camera/Camera.h"
#include "presentation/scene/nodes/light/Light.h"
#include "logic/input/InputHandler.h"
#include "presentation/scene/nodes/frameBuffer/FrameBuffer.h"

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

int main() {
    OpenGLInstance openGlInstance{};
    openGlInstance.init(SCR_WIDTH, SCR_HEIGHT);

    auto appState = std::make_unique<ApplicationState>();

    InputHandler inputHandler(appState);
    inputHandler.setupCallbacks(openGlInstance.getWindow());

    {
        GUI gui(openGlInstance.getWindow(), *appState);

        auto frameBuffer = std::make_unique<FrameBuffer>("Main");

//         TODO Do research on paths in C++. Try to be as OS agnostic as possible. Cerberus model had the 'windows slash' problem
        auto randomShader = std::make_unique<Shader>("random", IOUtils::getResource("shaders/basic/white.vert"),
                                                     IOUtils::getResource("shaders/debug/random.frag"));
        randomShader->setActive(false);
        frameBuffer->addChild(*randomShader);

        auto normalShader = std::make_unique<Shader>("normalDebug", IOUtils::getResource("shaders/debug/normalDebug.vert"),
                                               IOUtils::getResource("shaders/debug/normalDebug.geom"),
                                               IOUtils::getResource("shaders/debug/normalDebug.frag"));
        normalShader->setActive(false);
        randomShader->addChild(*normalShader);

        auto wireframeShader = std::make_unique<Shader>("wireframe", IOUtils::getResource("shaders/debug/normalDebug.vert"),
                                                     IOUtils::getResource("shaders/debug/wireframe.geom"),
                                                     IOUtils::getResource("shaders/debug/normalDebug.frag"));
        wireframeShader->setActive(false);
        normalShader->addChild(*wireframeShader);

        auto shader = std::make_unique<Shader>("blinnPhong", IOUtils::getResource("shaders/phong/basicBlinnPhong.vert"),
                                               IOUtils::getResource("shaders/phong/basicBlinnPhong.frag"));
        wireframeShader->addChild(*shader);

        auto camera = std::make_unique<Camera>("camera", SCR_WIDTH - gui.getGuiWidth(), SCR_HEIGHT, CameraMode::ANCHOR,
                                               glm::vec3(0.0f, 0.0f, 3.0f)); // TODO Set orientation to anchor
        appState->currentCamera = *camera;
        shader->addChild(*camera);

        auto pointLight = std::make_unique<Light>("pointLight", glm::vec3(1.0, 1.0f, 1.0f));
        camera->addChild(*pointLight);

        auto pointLightModel = ModelGenerator::generatePointLightRepresentation(pointLight);
        camera->addChild(*pointLightModel[0]);

        auto axis = ModelGenerator::generateAxis();
        pointLight->addChild(*axis[0]);

        AssetImporter assetImporter;
//        auto model = assetImporter.importModel(IOUtils::getResource("models/spitfire_mini/model/model.gltf"));
//        pointLight->addChild(*model[0]);

        appState->mainFrameBufferNode = *frameBuffer;
        appState->allNodes.push_back(std::move(frameBuffer));
        appState->allNodes.push_back(std::move(shader));
        appState->allNodes.push_back(std::move(camera));
        appState->allNodes.push_back(std::move(pointLight));
        appState->allNodes.insert(appState->allNodes.end(), std::make_move_iterator(pointLightModel.begin()),
                                  std::make_move_iterator(pointLightModel.end()));
        appState->allNodes.insert(appState->allNodes.end(), std::make_move_iterator(axis.begin()),
                                  std::make_move_iterator(axis.end()));
//        appState->allNodes.insert(appState->allNodes.end(), std::make_move_iterator(model.begin()),
//                                  std::make_move_iterator(model.end()));

        while (openGlInstance.isRunning()) {
            OpenGLInstance::pollEvents();
            gui.newFrame();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(gui.getGuiWidth(), 0, SCR_WIDTH - gui.getGuiWidth(), SCR_HEIGHT);
            RenderSceneVisitor renderSceneVisitor{};
            SceneNode::visitTree(*appState->mainFrameBufferNode, renderSceneVisitor);

            glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
            gui.render();

            openGlInstance.swapBuffers();
        }
    }

    glfwTerminate();
    return 0;
}
