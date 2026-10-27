#pragma once
#include <cstdint>

namespace ssrecomp::uwp {
// Logical Nintendo DS buttons; bit positions are owned by this adapter.
enum DsButton : std::uint16_t {
    A = 1u << 0, B = 1u << 1, Select = 1u << 2, Start = 1u << 3,
    Right = 1u << 4, Left = 1u << 5, Up = 1u << 6, Down = 1u << 7,
    R = 1u << 8, L = 1u << 9, X = 1u << 10, Y = 1u << 11
};

// Portable subset of Xbox controller state for the UWP input backend.
struct GamepadState {
    bool a = false, b = false, x = false, y = false;
    bool menu = false, view = false;
    bool dpad_up = false, dpad_down = false, dpad_left = false, dpad_right = false;
    bool left_shoulder = false, right_shoulder = false;
};

std::uint16_t MapGamepadToDsButtons(const GamepadState& state);
}
