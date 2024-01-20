#pragma once

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <game_window.h>
#include <gamepad/gamepad_ids.h>
#include <gamepad/gamepad.h>
#include <gamepad/joystick_manager.h>
#include <gamepad/gamepad_manager.h>

class WindowWithLinuxJoystick;

class LinuxGamepadJoystickManager {

private:
    bool initialized = false;
    std::unordered_set<WindowWithLinuxJoystick*> windows;
    WindowWithLinuxJoystick* focusedWindow;
    std::unordered_set<gamepad::Gamepad*> gamepads;
    std::shared_ptr<gamepad::JoystickManager> joystickManager;
    gamepad::GamepadManager gamepadManager;

    static GamepadButtonId mapButtonId(gamepad::GamepadButton id);
    static GamepadAxisId mapAxisId(gamepad::GamepadAxis id);

    void onGamepadState(gamepad::Gamepad* gp, bool connected);
    void onGamepadButton(gamepad::Gamepad* gp, gamepad::GamepadButton btn, bool state);
    void onGamepadAxis(gamepad::Gamepad* gp, gamepad::GamepadAxis axis, float value);

public:
    static LinuxGamepadJoystickManager instance;

    LinuxGamepadJoystickManager();

    void initialize();

    void loadMappingsFromFile(std::string const& path);

    void update(WindowWithLinuxJoystick* window);

    void addWindow(WindowWithLinuxJoystick* window);
    void removeWindow(WindowWithLinuxJoystick* window);

    void onWindowFocused(WindowWithLinuxJoystick* window, bool focused);

};