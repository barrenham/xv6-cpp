#pragma once
#include "kernel/io/keyboard.h"

namespace fbterm
{
// US keyboard layout. Modifier state belongs to this terminal, not the kernel.
class Keyboard
{
public:
  unsigned translate(const KeyboardEvent &event, char (&output)[8])
  {
    if (event.type == XV_KEY_SYN && event.code == XV_KEY_SYN_DROPPED)
    {
      reset();
      return 0;
    }
    if (event.type != XV_KEY_EVENT || event.code >= 128 || event.value > 2)
    {
      return 0;
    }
    unsigned code = event.code;
    bool was_pressed = pressed_[code];
    pressed_[code] = event.value != 0;
    if (!event.value)
    {
      return 0;
    }
    if (code == 58)
    {
      if (event.value == 1 && !was_pressed)
      {
        caps_ = !caps_;
      }
      return 0;
    }
    if (code == 42 || code == 54 || code == 29 || code == 97 || code == 56 || code == 100)
    {
      return 0;
    }

    bool shift = pressed_[42] || pressed_[54];
    bool control = pressed_[29] || pressed_[97];
    bool alt = pressed_[56] || pressed_[100];
    unsigned used = 0;
    if (alt)
    {
      output[used++] = 27;
    }
    const char *sequence = nullptr;
    switch (code)
    {
    case 102:
      sequence = "\033[H";
      break;
    case 103:
      sequence = "\033[A";
      break;
    case 104:
      sequence = "\033[5~";
      break;
    case 105:
      sequence = "\033[D";
      break;
    case 106:
      sequence = "\033[C";
      break;
    case 107:
      sequence = "\033[F";
      break;
    case 108:
      sequence = "\033[B";
      break;
    case 109:
      sequence = "\033[6~";
      break;
    case 110:
      sequence = "\033[2~";
      break;
    case 111:
      sequence = "\033[3~";
      break;
    }
    if (sequence)
    {
      while (*sequence)
      {
        output[used++] = *sequence++;
      }
      return used;
    }

    char value = 0;
    bool mapped = true;
    if (code >= 2 && code <= 13)
    {
      value = (shift ? "!@#$%^&*()_+" : "1234567890-=")[code - 2];
    }
    else if (code >= 16 && code <= 27)
    {
      value = (shift ? "QWERTYUIOP{}" : "qwertyuiop[]")[code - 16];
    }
    else if (code >= 30 && code <= 41)
    {
      value = (shift ? "ASDFGHJKL:\"~" : "asdfghjkl;'`")[code - 30];
    }
    else if (code >= 44 && code <= 53)
    {
      value = (shift ? "ZXCVBNM<>?" : "zxcvbnm,./")[code - 44];
    }
    else
    {
      switch (code)
      {
      case 1:
        value = 27;
        break;
      case 14:
        value = 127;
        break;
      case 15:
        value = '\t';
        break;
      case 28:
      case 96:
        value = '\r';
        break;
      case 43:
        value = shift ? '|' : '\\';
        break;
      case 57:
        value = ' ';
        break;
      default:
        mapped = false;
        break;
      }
    }
    if (!mapped)
    {
      return 0;
    }
    bool letter = (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z');
    if (caps_ && letter)
    {
      value ^= 32;
    }
    if (control)
    {
      if (letter)
      {
        value &= 31;
      }
      else if (value == ' ' || value == '@' || code == 3)
      {
        value = 0;
      }
      else if (value >= '[' && value <= '_')
      {
        value &= 31;
      }
      else if (code == 7)
      {
        value = 30;
      }
      else if (code == 12)
      {
        value = 31;
      }
    }
    output[used++] = value;
    return used;
  }

private:
  bool pressed_[128]{};
  bool caps_ = false;

  void reset()
  {
    for (auto &pressed : pressed_)
    {
      pressed = false;
    }
    caps_ = false;
  }
};
} // namespace fbterm
