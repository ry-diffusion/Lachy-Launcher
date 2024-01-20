#include "joystick_manager_glfw.h"

#include <cstring>
#include <fstream>
#include "window_glfw.h"

std::unordered_set<GLFWGameWindow*> GLFWJoystickManager::windows;
GLFWGameWindow* GLFWJoystickManager::focusedWindow;
std::unordered_map<int, GLFWJoystickManager::JoystickInfo> GLFWJoystickManager::connectedJoysticks;
std::unordered_set<int> GLFWJoystickManager::userIds;

void GLFWJoystickManager::init() {
    glfwSetJoystickCallback(_glfwJoystickCallback);
    loadMappingsFromFile("gamecontrollerdb.txt");
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
        if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i))
            _glfwJoystickCallback(i, GLFW_CONNECTED);
    }
}

void GLFWJoystickManager::loadMappingsFromFile(std::string const& path) {
    std::ifstream fs(path);
    if (!fs)
        return;
    std::string line;
    while (std::getline(fs, line)) {
        if (!line.empty())
            glfwUpdateGamepadMappings(line.c_str());
    }
}

int GLFWJoystickManager::nextUnassignedUserId() {
    for (int i = 0; ; i++) {
        if (userIds.count(i) <= 0)
            return i;
    }
}

void GLFWJoystickManager::update(GLFWGameWindow* window) {
    if (focusedWindow != window)
        return;

    for (auto& j : connectedJoysticks) {
        GLFWgamepadstate state;
        glfwGetGamepadState(j.first, &state);
        for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
            if (state.buttons[i] != j.second.oldButtonStates[i]) {
                window->onGamepadButton(j.second.userId, mapButtonId(i), state.buttons[i] != 0);
            }
        }
        for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
            window->onGamepadAxis(j.second.userId, mapAxisId(i), state.axes[i]);
        }

        memcpy(j.second.oldButtonStates, state.buttons, GLFW_GAMEPAD_BUTTON_LAST + 1);
    }
}

void GLFWJoystickManager::update() {
    update(focusedWindow);
}

void GLFWJoystickManager::addWindow(GLFWGameWindow* window) {
    windows.insert(window);
    for (auto& joystick : connectedJoysticks)
        window->onGamepadState(joystick.second.userId, true);
}

void GLFWJoystickManager::removeWindow(GLFWGameWindow* window) {
    windows.erase(window);
}

void GLFWJoystickManager::onWindowFocused(GLFWGameWindow* window, bool focused) {
    if (focused)
        focusedWindow = window;
    else if (focusedWindow == window /* && !focused */)
        focusedWindow = nullptr;
}

void GLFWJoystickManager::_glfwJoystickCallback(int joystick, int action) {
    if (!glfwJoystickIsGamepad(joystick))
        return;

    auto js = connectedJoysticks.find(joystick);
    int userId;
    if (action == GLFW_CONNECTED) {
        if (js != connectedJoysticks.end())
            return;
        userId = nextUnassignedUserId();
        userIds.insert(userId);
        connectedJoysticks.insert({joystick, JoystickInfo(userId)});
    } else if (action == GLFW_DISCONNECTED) {
        if (js == connectedJoysticks.end())
            return;
        userId = js->second.userId;
        userIds.erase(userId);
        connectedJoysticks.erase(joystick);
    }

    for (GLFWGameWindow* window : windows)
        window->onGamepadState(userId, action == GLFW_CONNECTED);
}

GamepadButtonId GLFWJoystickManager::mapButtonId(int id) {
    switch (id) {
        case GLFW_GAMEPAD_BUTTON_A: return GamepadButtonId::A;
        case GLFW_GAMEPAD_BUTTON_B: return GamepadButtonId::B;
        case GLFW_GAMEPAD_BUTTON_X: return GamepadButtonId::X;
        case GLFW_GAMEPAD_BUTTON_Y: return GamepadButtonId::Y;
        case GLFW_GAMEPAD_BUTTON_BACK: return GamepadButtonId::BACK;
        case GLFW_GAMEPAD_BUTTON_START: return GamepadButtonId::START;
        case GLFW_GAMEPAD_BUTTON_GUIDE: return GamepadButtonId::GUIDE;
        case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: return GamepadButtonId::LB;
        case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: return GamepadButtonId::RB;
        case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: return GamepadButtonId::LEFT_STICK;
        case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: return GamepadButtonId::RIGHT_STICK;
        case GLFW_GAMEPAD_BUTTON_DPAD_UP: return GamepadButtonId::DPAD_UP;
        case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: return GamepadButtonId::DPAD_RIGHT;
        case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: return GamepadButtonId::DPAD_DOWN;
        case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: return GamepadButtonId::DPAD_LEFT;
        default: return GamepadButtonId::UNKNOWN;
    }
}

GamepadAxisId GLFWJoystickManager::mapAxisId(int id) {
    switch (id) {
        case GLFW_GAMEPAD_AXIS_LEFT_X: return GamepadAxisId::LEFT_X;
        case GLFW_GAMEPAD_AXIS_LEFT_Y: return GamepadAxisId::LEFT_Y;
        case GLFW_GAMEPAD_AXIS_RIGHT_X: return GamepadAxisId::RIGHT_X;
        case GLFW_GAMEPAD_AXIS_RIGHT_Y: return GamepadAxisId::RIGHT_Y;
        case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER: return GamepadAxisId::LEFT_TRIGGER;
        case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER: return GamepadAxisId::RIGHT_TRIGGER;
        default: return GamepadAxisId::UNKNOWN;
    }
}