#pragma once

#include <game_window.h>

class WindowWithLinuxJoystick : public GameWindow {

private:
    friend class LinuxGamepadJoystickManager;

public:
    WindowWithLinuxJoystick(std::string const& title, int width, int height, GraphicsApi api);

    ~WindowWithLinuxJoystick() override;

    void addWindowToGamepadManager();

    void updateGamepad();

};