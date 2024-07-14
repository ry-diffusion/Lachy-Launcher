#pragma once

#include "Color.h"
#include "minecraft/std/string.h"

class Font
{
 public:
  /// @symbol _ZN4Font10drawShadowERKSsffRK5Colorb
  void drawShadow(mcpe::string& text, float x, float y, Color& color);
};