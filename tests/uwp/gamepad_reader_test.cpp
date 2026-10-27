#include "../../frontend/uwp/gamepad_reader.h"
#include <cassert>

int main()
{
#if !defined(_WIN32)
    ssrecomp::uwp::GamepadState state;
    state.a = true;
    assert(!ssrecomp::uwp::ReadXboxGamepad(state));
    assert(!state.a);
#endif
    return 0;
}
