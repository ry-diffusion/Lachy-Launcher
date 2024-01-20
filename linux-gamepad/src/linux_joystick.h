#pragma once

#include <gamepad/joystick.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <vector>
#include <bitset>

namespace gamepad {

class LinuxJoystickManager;

class LinuxJoystick : public Joystick {

private:
    LinuxJoystickManager* mgr;
    struct libevdev* edev;
    std::string devPath;

    struct AxisInfo {
        int index;
        int min, max;
        int flat, fuzz;
    };

    int buttons[KEY_CNT];
    AxisInfo axis[ABS_CNT];

    static constexpr int BUTTON_COUNT = KEY_CNT;
    std::bitset<BUTTON_COUNT> buttonValues;
    static constexpr int AXIS_COUNT = ABS_CNT;
    float axisValues[AXIS_COUNT];
    static constexpr int HAT_COUNT = (ABS_HAT3Y + 1 - ABS_HAT0X) / 2;
    int hatValues[HAT_COUNT];

    inline bool isHat(int index) {
        return (index >= ABS_HAT0X && index <= ABS_HAT3Y);
    }

public:
    LinuxJoystick(LinuxJoystickManager* mgr, std::string const& path, struct libevdev* edev);

    inline std::string const& getPath() const {
        return devPath;
    }

    void poll();

    std::string getGUID() const override;

    bool getButton(int index) const override {
        if (index < 0 || index >= BUTTON_COUNT)
            return false;
        return buttonValues[index];
    }

    float getAxis(int index) const override {
        if (index < 0 || index >= AXIS_COUNT)
            return false;
        return axisValues[index];
    }

    int getHat(int index) const override {
        if (index < 0 || index >= HAT_COUNT)
            return false;
        return hatValues[index];
    }

};

}