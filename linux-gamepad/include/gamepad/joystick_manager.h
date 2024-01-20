#pragma once

#include <functional>
#include "callback_list.h"

namespace gamepad {

class Joystick;

class JoystickManager {

public:
    CallbackList<std::function<void (Joystick* js)>> onJoystickConnected;
    CallbackList<std::function<void (Joystick* js)>> onJoystickDisconnected;
    CallbackList<std::function<void (Joystick* js, int button, bool state)>> onJoystickButton;
    CallbackList<std::function<void (Joystick* js, int axis, float value)>> onJoystickAxis;
    CallbackList<std::function<void (Joystick* js, int hat, int value)>> onJoystickHat;

    virtual void initialize() = 0;

    virtual void poll() = 0;

};

}