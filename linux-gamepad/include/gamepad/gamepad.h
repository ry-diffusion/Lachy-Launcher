#pragma once

#include "gamepad_ids.h"

namespace gamepad {

class Joystick;
class GamepadMapping;

class Gamepad {

private:
    int index;
    Joystick& joystick;
    GamepadMapping& mapping;

public:
    Gamepad(int index, Joystick& joystick, GamepadMapping& mapping) : index(index), joystick(joystick),
                                                                      mapping(mapping) {}

    Joystick const& getJoystick() const {
        return joystick;
    }

    GamepadMapping& getMapping() const {
        return mapping;
    }

    int getIndex() const {
        return index;
    }

    bool getButton(GamepadButton index) const;

    float getAxis(GamepadAxis index) const;

};

}