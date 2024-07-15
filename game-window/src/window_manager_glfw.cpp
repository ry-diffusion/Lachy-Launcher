#include "window_manager_glfw.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

#include "joystick_manager_glfw.h"
#include "window_glfw.h"

GLFWWindowManager::GLFWWindowManager()
{
  if (glfwInit() != GLFW_TRUE)
  {
    const char *description;
    (void)glfwGetError(&description);

    throw std::runtime_error("An error occurred while initializing GLFW: " +
                             std::string(description));
  }

  GLFWJoystickManager::init();
}

GameWindowManager::ProcAddrFunc GLFWWindowManager::getProcAddrFunc()
{
  return (GameWindowManager::ProcAddrFunc)glfwGetProcAddress;
}

std::shared_ptr<GameWindow> GLFWWindowManager::createWindow(
    const std::string &title, int width, int height, GraphicsApi api)
{
  return std::make_shared<GLFWGameWindow>(title, width, height, api);
}

void GLFWWindowManager::addGamepadMappingFile(const std::string &path)
{
  GLFWJoystickManager::loadMappingsFromFile(path);
}

// Define this window manager as the used one
std::shared_ptr<GameWindowManager> GameWindowManager::createManager()
{
  return std::make_shared<GLFWWindowManager>();
}