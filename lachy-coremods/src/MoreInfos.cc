#include <coremod.h>
#include <game_window_manager.h>
#include <minecraft/Screen.h>
#include <minecraft/SharedConstants.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "log.h"

void measureMemoryUsage(size_t &rss, size_t &virt)
{
  std::ifstream file("/proc/self/statm");

  file >> virt >> rss;

  rss *= getpagesize();
  virt *= getpagesize();
}

class MoreInfos : public CoreMod
{
  bool isHudOpen = false;
  GenericMinecraft *mc = nullptr;
  std::shared_ptr<GameWindow> gameWindow;
  const char* TAG = "MoreInfos";

  void onLoad() override
  {
    Log::info(TAG, "Mod Loaded!");
  }

  void onStart(GenericMinecraft *genericMinecraft) override
  {
    mc = genericMinecraft;
  }

  void onGameWindowCreated(const std::shared_ptr<GameWindow> &window) override
  {
    gameWindow = window;
  }

  void onGUIRequested() override
  {
    if (!gameWindow || !mc || !isHudOpen) return;
    const auto white = Color(1.0f, 1.0f, 1.0f, 1.0f);
    size_t virt, rss;

    measureMemoryUsage(rss, virt);
    std::stringstream ss;

    ss << "Lachy - Minecraft " << *SharedConstants::MajorVersion << "."
       << *SharedConstants::MinorVersion << "."
       << *SharedConstants::PatchVersion;
    ss << " (PRESS F3 TO QUIT)" << std::endl;

    const auto windowManager = GameWindowManager::getManager();
    ss << "FPS: " << gameWindow->fps << std::endl;
    ss << "Memory: " << rss / 1024 / 1024 << "MB / " << virt / 1024 / 1024
       << "MB" << std::endl;

    const auto glGetString = reinterpret_cast<const char *(*)(int)>(
        windowManager->getProcAddrFunc()("glGetString"));

    ss << "Renderer: " << glGetString(0x1F01) << " (" << glGetString(0x1F00)
       << ")" << std::endl;
    ss << "GL Version: " << glGetString(0x1F02) << std::endl;
    ss << "GLSL Version: " << glGetString(0x8B8C) << std::endl;

    auto text = mcpe::string(ss.str());
    const auto font = mc->getFont();

    font->drawTransformed(text, 0.0f, 0.0f, white, 0.0f, 100.0f, false, 6.0f);

    Screen::tick();
  }

  void onKeyboardInput(const int key, const int action) override
  {
    // F3 Key
    if (key == 111 + 3 && 0 == action)
    {
      isHudOpen = !isHudOpen;
    }
  }
};

CoreMod *Instance = new MoreInfos();