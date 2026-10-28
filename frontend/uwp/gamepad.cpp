#include "gamepad.h"

namespace ssrecomp::uwp {
std::uint16_t MapGamepadToDsButtons(const GamepadState& s)
{
    std::uint16_t result = 0;
    if (s.a) result |= A;
    if (s.b) result |= B;
    if (s.x) result |= X;
    if (s.y) result |= Y;
    if (s.menu) result |= Start;
    if (s.view) result |= Select;
    if (s.dpad_up) result |= Up;
    if (s.dpad_down) result |= Down;
    if (s.dpad_left) result |= Left;
    if (s.dpad_right) result |= Right;
    if (s.left_shoulder) result |= L;
    if (s.right_shoulder) result |= R;
    return result;
}
}
