#pragma once

#include <functional>
#include <vector>
#include <set>
#include "gamepad_mapping.h"
#include "callback_list.h"

namespace gamepad {

class JoystickManager;
class Gamepad;

class GamepadManager : public CallbackAutoHandler {

public:
    GamepadManager(JoystickManager& jsManager);

    CallbackList<std::function<void (Gamepad* gp)>> onGamepadConnected;
    CallbackList<std::function<void (Gamepad* gp)>> onGamepadDisconnected;
    CallbackList<std::function<void (Gamepad* gp, GamepadButton btn, bool pressed)>> onGamepadButton;
    CallbackList<std::function<void (Gamepad* gp, GamepadAxis axis, float value)>> onGamepadAxis;

    void addMapping(GamepadMapping& mapping);
    void addMapping(std::string const& mapping);

protected:
    JoystickManager& jsManager;
    GamepadMapping defaultMapping;
    std::unordered_map<std::string, GamepadMapping> mappings;
    std::set<int> takenGamepadIds;
    int takenGamepadLowId = 0;

    int takeGamepadId();
    void putGamepadIdBack(int i);

    GamepadMapping& getMapping(Joystick* js);

    void onJoystickConnected(Joystick* js);
    void onJoystickDisconnected(Joystick* js);

    void onJoystickButton(Joystick* js, int button, bool state);
    void onJoystickAxis(Joystick* js, int axis, float value);
    void onJoystickHat(Joystick* js, int hat, int value);

};

}