#pragma once
#include <cassert>

#include "Font.h"
#include "ClientInstance.h"
#include "MinecraftClient.h"
#include "MinecraftGame.h"

class GenericMinecraft
{
 public:
  GenericMinecraft(MinecraftGame* game, MinecraftClient* client)
  {
    if (game)
    {
      isModern = true;
      this->game = game;
    }

    if (client)
    {
      isModern = false;
      this->client = client;
    }

    assert(game != nullptr || client != nullptr);
  }

  Font* getFont()
  {
      return font;
  }

  void setRenderingSize(int width, int height)
  {
    if (isModern)
      game->setRenderingSize(width, height);
    else
      client->setRenderingSize(width, height);
  }
  void setUISizeAndScale(int width, int height, float scale)
  {
    if (isModern)
      game->setUISizeAndScale(width, height, scale);
    else
      client->setUISizeAndScale(width, height, scale);
  };

public:
  ClientInstance *instance = nullptr;
  Font* font = nullptr;
 private:
  MinecraftClient* client = nullptr;
  MinecraftGame* game = nullptr;

  bool isModern = false;
};
