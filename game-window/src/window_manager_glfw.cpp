#include <stdexcept>
#include "window_manager_glfw.h"
#include "window_glfw.h"
#include "joystick_manager_glfw.h"

GLFWWindowManager::GLFWWindowManager() {
    if (glfwInit() != GLFW_TRUE)
        throw std::runtime_error("glfwInit error");
    GLFWJoystickManager::init();
}

GameWindowManager::ProcAddrFunc GLFWWindowManager::getProcAddrFunc() {
    return (GameWindowManager::ProcAddrFunc) glfwGetProcAddress;
}

std::shared_ptr<GameWindow> GLFWWindowManager::createWindow(const std::string& title, int width, int height,
                                                             GraphicsApi api) {
    return std::shared_ptr<GameWindow>(new GLFWGameWindow(title, width, height, api));
}

void GLFWWindowManager::addGamepadMappingFile(const std::string &path) {
    GLFWJoystickManager::loadMappingsFromFile(path);
}


// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager() {
    return std::shared_ptr<GameWindowManager>(new GLFWWindowManager());
}