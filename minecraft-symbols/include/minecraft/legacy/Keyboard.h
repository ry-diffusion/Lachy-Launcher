#pragma once

#include <vector>

#include "../Keyboard.h"
#include "../std/string.h"

namespace Legacy
{

  namespace Pre_1_2
  {

    class Keyboard
    {
     public:
      struct InputEvent
      {
        int event;
        unsigned int key;
        InputEvent(::Keyboard::InputEvent const& ev)
            : event(ev.event), key(ev.key)
        {
        }
      };

      /// @symbol _ZN8Keyboard7_inputsE
      static std::vector<Legacy::Pre_1_2::Keyboard::InputEvent>* _inputs;
    };

  }  // namespace Pre_1_2

  namespace Pre_0_17
  {

    class Keyboard
    {
     public:
      /// @symbol _ZN8Keyboard8feedTextERKSsb
      static void feedText(mcpe::string const&, bool);
    };

  }  // namespace Pre_0_17

}  // namespace Legacy