
#pragma once
#include <minecraft/GenericMinecraft.h>

#include <iostream>
#include <utility>

class GameWindow;
class CoreMod
{
 public:
  virtual ~CoreMod() = default;
  virtual void onLoad() = 0;
  virtual void onCreate(void* handle)
  {
  }

  virtual void onStart(GenericMinecraft* genericMinecraft)
  {
  }

  virtual void onGUIRequested()
  {

  }

  virtual void onKeyboardInput(int key, int action)
  {

  }

  virtual void onGameWindowCreated(const std::shared_ptr<GameWindow>& window)
  {
  }
};

extern CoreMod *Instance;

extern "C" void coremod_onLoad()
{
  Instance->onLoad();
}

extern "C" void coremod_onCreate(void *handle)
{
  Instance->onCreate(handle);
}

extern "C" void coremod_onStart(GenericMinecraft *genericMinecraft)
{
  Instance->onStart(genericMinecraft);
}

extern "C" void coremod_onGUIRequested()
{
  Instance->onGUIRequested();
}

extern "C" void coremod_onKeyboardInput(int key, int action)
{
  Instance->onKeyboardInput(key, action);
}

extern "C" void coremod_onGameWindowCreated(const std::shared_ptr<GameWindow>& window)
{
  Instance->onGameWindowCreated(window);
}

