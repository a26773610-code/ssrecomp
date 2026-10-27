#include "gamepad_reader.h"

#if defined(_WIN32)
#include <winapifamily.h>
#endif

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#include <winrt/base.h>
#include <winrt/Windows.Gaming.Input.h>

namespace ssrecomp::uwp {
bool ReadXboxGamepad(GamepadState& state)
{
    state = {};
    const auto pads = winrt::Windows::Gaming::Input::Gamepad::Gamepads();
    if (pads.Size() == 0) return false;
    const auto reading = pads.GetAt(0).GetCurrentReading();
    using Buttons = winrt::Windows::Gaming::Input::GamepadButtons;
    const auto pressed = [&](Buttons button) {
        return (reading.Buttons & button) == button;
    };
    state.a = pressed(Buttons::A);
    state.b = pressed(Buttons::B);
    state.x = pressed(Buttons::X);
    state.y = pressed(Buttons::Y);
    state.menu = pressed(Buttons::Menu);
    state.view = pressed(Buttons::View);
    state.dpad_up = pressed(Buttons::DPadUp);
    state.dpad_down = pressed(Buttons::DPadDown);
    state.dpad_left = pressed(Buttons::DPadLeft);
    state.dpad_right = pressed(Buttons::DPadRight);
    state.left_shoulder = pressed(Buttons::LeftShoulder);
    state.right_shoulder = pressed(Buttons::RightShoulder);
    state.left_x = static_cast<float>(reading.LeftThumbstickX);
    state.left_y = static_cast<float>(reading.LeftThumbstickY);
    state.right_x = static_cast<float>(reading.RightThumbstickX);
    state.right_y = static_cast<float>(reading.RightThumbstickY);
    state.left_trigger = static_cast<float>(reading.LeftTrigger);
    state.right_trigger = static_cast<float>(reading.RightTrigger);
    return true;
}
}
#else
namespace ssrecomp::uwp {
bool ReadXboxGamepad(GamepadState& state)
{
    state = {};
    return false;
}
}
#endif
