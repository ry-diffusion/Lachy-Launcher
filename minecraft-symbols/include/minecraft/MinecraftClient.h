#pragma once
#include "minecraft/Font.h"

class MinecraftClient
{
 public:
  /// @symbol _ZNK15MinecraftClient7getFontEv
  Font* getFont() ;

  /// @symbol _ZN15MinecraftClient16setRenderingSizeEii
  void setRenderingSize(int width, int height) ;

  /// @symbol _ZN15MinecraftClient17setUISizeAndScaleEiif
  void setUISizeAndScale(int width, int height, float scale) ;
};