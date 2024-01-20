#pragma once

#include "game_window.h"
#include <memory>

class GameWindowManager {

private:
    static std::shared_ptr<GameWindowManager> instance;

    static std::shared_ptr<GameWindowManager> createManager();

public:
    static std::shared_ptr<GameWindowManager> getManager();


    using AnyFunc = void* (*)();
    using ProcAddrFunc = AnyFunc (*)(const char*);


    virtual ProcAddrFunc getProcAddrFunc() = 0;

    virtual std::shared_ptr<GameWindow>
    createWindow(const std::string& title, int width, int height, GraphicsApi api) = 0;

    virtual void addGamepadMappingFile(const std::string& path) = 0;

};