#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/BillboardShader.hpp"
#include "Shaders/VolumeShader.hpp"
#include "Shaders/PointShader.hpp"

#include "Cloud.hpp"

/* Initial values */

std::string RESOURCE_DIR = "../res/";

/* Light position */
glm::vec3 lightPos;

/* Shaders */
BillboardShader *billboardShader;
VolumeShader *volumeShader;
PointShader *pointShader;

// TODO : imgui
void takeInput() {
    /* Update light */
    if (Keyboard::isKeyPressed(GLFW_KEY_Z)) {
        lightPos.x += 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_X)) {
        lightPos.x -= 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_C)) {
        lightPos.y += 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_V)) {
        lightPos.y -= 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_B)) {
        lightPos.z += 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_N)) {
        lightPos.z -= 10.f;
    }
}

int main() {
    /* Init window, keyboard, and mouse wrappers */
    if (Window::init("Clouds")) {
        std::cerr << "ERROR" << std::endl;
        return 1;
    }

    /* Create light */
    lightPos = glm::vec3(100.f, 100.f, 100.f);

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    billboardShader->init(RESOURCE_DIR + "cloud.png", RESOURCE_DIR + "cloudMap.png");
    volumeShader = new VolumeShader(RESOURCE_DIR + "voxelize_vert.glsl", RESOURCE_DIR + "voxelize_frag.glsl");
    volumeShader->init(16, glm::vec2(-20.f, 20.f), glm::vec2(-2.f, 15.f), glm::vec2(12.f, 12.f));
//    pointShader = new PointShader(RESOURCE_DIR + "point_vert.glsl", RESOURCE_DIR + "point_frag.glsl");
//    pointShader->init();

    /* Init rendering */
    GLSL::checkVersion();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_BLEND));
    CHECK_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* First render pass - generate volume */
    Window::update();
    Camera::update(Window::timeStep);
    CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    CHECK_GL_CALL(glClearColor(0.f, 0.4f, 0.7f, 1.f));

    billboardShader->addCloud(glm::vec3(5.f, 0.f, 0.f));
    while (!Window::shouldClose()) {
        /* Update context */
        Window::update();

        /* Update camera */
        Camera::update(Window::timeStep);

        takeInput();

        /* Render */
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        CHECK_GL_CALL(glClearColor(0.f, 0.4f, 0.7f, 1.f));
        volumeShader->voxelize(billboardShader->quad, glm::vec3(5.f, 0.f, 0.f));
        //billboardShader->render(lightPos);
        //pointShader->render(volumeShader->getVoxelData());
    }
}
