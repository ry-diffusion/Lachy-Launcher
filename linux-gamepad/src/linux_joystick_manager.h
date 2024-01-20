#pragma once

#include <gamepad/joystick_manager.h>
#include <memory>
#include <libudev.h>
#include "linux_joystick.h"

namespace gamepad {

class LinuxJoystickManager : public JoystickManager {

private:
    struct udev* udev = nullptr;
    struct udev_monitor* udevMonitor = nullptr;
    int udevMonitorFd;
    std::vector<std::unique_ptr<LinuxJoystick>> joysticks;

public:
    LinuxJoystickManager() {}
    ~LinuxJoystickManager();

    void initialize() override;

    void poll() override;

    void onDeviceAdded(struct udev_device* dev);
    void onDeviceRemoved(struct udev_device* dev);

};

}