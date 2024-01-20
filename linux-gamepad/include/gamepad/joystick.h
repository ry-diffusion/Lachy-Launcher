#pragma once

#include <string>

namespace gamepad {

class Gamepad;

class Joystick {

private:
    Gamepad* gamepad = nullptr;

public:
    virtual std::string getGUID() const = 0;

    virtual bool getButton(int index) const = 0;

    virtual float getAxis(int index) const = 0;

    virtual int getHat(int index) const = 0;


    Gamepad* getGamepad() const {
        return gamepad;
    }

    void setGamepad(Gamepad* gamepad) {
        this->gamepad = gamepad;
    }

};

}