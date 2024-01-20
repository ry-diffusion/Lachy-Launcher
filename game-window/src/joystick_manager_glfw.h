#pragma once

#include <unordered_set>
#include <unordered_map>
#include <GLFW/glfw3.h>
#include <game_window.h>

class GLFWGameWindow;

class GLFWJoystickManager {

private:
    struct JoystickInfo {
        int userId;
        char oldButtonStates[GLFW_GAMEPAD_BUTTON_LAST + 1];

        JoystickInfo(int id) : userId(id) {}
    };

    static std::unordered_set<GLFWGameWindow*> windows;
    static GLFWGameWindow* focusedWindow;
    static std::unordered_map<int, JoystickInfo> connectedJoysticks;
    static std::unordered_set<int> userIds;

    static int nextUnassignedUserId();

    static void _glfwJoystickCallback(int joystick, int action);

    static GamepadButtonId mapButtonId(int id);
    static GamepadAxisId mapAxisId(int id);

public:
    static void init();
    static void loadMappingsFromFile(std::string const& path);

    static void update();
    static void update(GLFWGameWindow* window);

    static void addWindow(GLFWGameWindow* window);
    static void removeWindow(GLFWGameWindow* window);

    static void onWindowFocused(GLFWGameWindow* window, bool focused);

};