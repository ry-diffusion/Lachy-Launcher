#pragma once

#include "game_window_manager.h"

class EGLUTWindowManager : public GameWindowManager {

public:
    EGLUTWindowManager();

    ProcAddrFunc getProcAddrFunc() override;

    std::shared_ptr<GameWindow> createWindow(const std::string& title, int width, int height, GraphicsApi api) override;

    void addGamepadMappingFile(const std::string& path) override;

};