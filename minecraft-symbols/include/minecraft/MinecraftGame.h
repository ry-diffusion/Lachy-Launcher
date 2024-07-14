#pragma once

#include <memory>

#include "App.h"
#include "std/function.h"
#include "Font.h"
class Options;
class ClientInstance;

namespace BedrockEngine
{
  struct AppIsland
  {
    void* appIsland_vtable;
  };
};  // namespace BedrockEngine
class MinecraftGame : public BedrockEngine::AppIsland
{
 public:
  //char filler[0x4000];



  bool isInGame() const;

  void requestLeaveGame(bool, bool);

  /// @symbol _ZN13MinecraftGame24doPrimaryClientReadyWorkESt8functionIFvvEE
  void doPrimaryClientReadyWork(mcpe::function<void()>);

  std::shared_ptr<Options> getPrimaryUserOptions();

  ClientInstance* getPrimaryClientInstance();

  void startLeaveGame();

  void continueLeaveGame();

  void setTextboxText(mcpe::string const&, int);

  /// @symbol _ZN13MinecraftGame16setRenderingSizeEii
  void setRenderingSize(int width, int height) ;

  /// @symbol _ZN13MinecraftGame17setUISizeAndScaleEiif
  void setUISizeAndScale(int width, int height, float scale) ;
};
