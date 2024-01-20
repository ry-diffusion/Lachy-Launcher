#include <game_window_manager.h>

std::shared_ptr<GameWindowManager> GameWindowManager::instance;

std::shared_ptr<GameWindowManager> GameWindowManager::getManager() {
    if (!instance)
        instance = createManager();
    return instance;
}