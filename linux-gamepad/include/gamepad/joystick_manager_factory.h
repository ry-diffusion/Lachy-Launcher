#pragma once

#include <memory>

namespace gamepad {

class JoystickManager;

class JoystickManagerFactory {

public:
    static std::shared_ptr<JoystickManager> create();

};

}