#pragma once
#include "gamepad.h"

namespace ssrecomp::uwp {
// Polls the first available Xbox controller via Windows.Gaming.Input.
// Returns false when no controller is connected or on non-UWP platforms.
bool ReadXboxGamepad(GamepadState& state);
}
