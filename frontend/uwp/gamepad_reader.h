#pragma once
#include "gamepad.h"

namespace ssrecomp::uwp {
// Polls the first available Xbox controller via Windows.Gaming.Input.
// Returns false when no controller is connected or on non-UWP platforms.
bool ReadXboxGamepad(GamepadState& state);
// Ensure Xbox B/BackRequested cannot close the UWP game while B is
// used as Nintendo DS Cancel. Call after the SDL WinRT window exists.
void InstallXboxBackHandler();
}
