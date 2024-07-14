#pragma once

#include <memory>

#include "Font.h"

namespace Social
{
  class User;
}

class ClientInstance
{
 public:
  std::shared_ptr<Social::User> getUser();

  void _startLeaveGame();

  void _syncDestroyGame();

    /// @symbol _ZNK14ClientInstance7getFontEv
  Font* getFont() const;
};