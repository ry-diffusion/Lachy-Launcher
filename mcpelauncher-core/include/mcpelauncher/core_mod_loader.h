#pragma once

#include <minecraft/GenericMinecraft.h>

#include <memory>
#include <set>
#include <string>
#include <vector>
class GameWindow;
class CoreModLoader
{
 private:
  std::vector<void*> mods;

  static std::shared_ptr<CoreModLoader> m_Instance;

  static std::vector<std::string> getModDependencies(std::string const& path);

  void loadModMulti(std::string const& path, std::string const& fileName,
                    std::set<std::string>& otherMods);

 public:
  static std::shared_ptr<CoreModLoader> getInstance();
  static void* loadMod(std::string const& path);

  void loadModsFromDirectory(std::string const& path);

  void onCreate(void*handle);
  void onStart(GenericMinecraft *genericMinecraft);
  void onGUIRequested();
  void onKeyboardInput(int key, int action);
  void onGameWindowCreated(std::shared_ptr<GameWindow> gameWindow);
};