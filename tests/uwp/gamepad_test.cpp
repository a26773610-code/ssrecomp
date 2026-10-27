#include "../../frontend/uwp/gamepad.h"
#include <cassert>
using namespace ssrecomp::uwp;
int main()
{
    assert(MapGamepadToDsButtons({}) == 0);
    GamepadState s;
    s.a = true; s.menu = true; s.dpad_up = true; s.left_shoulder = true;
    assert(MapGamepadToDsButtons(s) == (A | Start | Up | L));
    s = {}; s.b = true; s.view = true; s.dpad_right = true; s.right_shoulder = true;
    assert(MapGamepadToDsButtons(s) == (B | Select | Right | R));
    s = {}; s.x = true; s.y = true; s.dpad_left = true; s.dpad_down = true;
    assert(MapGamepadToDsButtons(s) == (X | Y | Left | Down));
}
