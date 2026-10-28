// Xbox UWP visual/runtime diagnostic. This is NOT the SoulSilver game.
// Draws a blue screen via DXGI CoreWindow and changes color on controller input.
// Built separately so packaging can be verified without copyrighted game data.
#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Gaming.Input.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <windows.h>
#include <cstdint>

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::Gaming::Input;
using Microsoft::WRL::ComPtr;

struct DiagnosticView final : implements<DiagnosticView, IFrameworkView>
{
    CoreWindow window{nullptr};
    bool closed = false;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain1> swapchain;
    ComPtr<ID3D11RenderTargetView> renderTarget;

    void Initialize(CoreApplicationView const&) {}
    void SetWindow(CoreWindow const& value)
    {
        window = value;
        window.Closed([this](CoreWindow const&, CoreWindowEventArgs const&) {
            closed = true;
        });
    }
    void Load(hstring const&)
    {
        const D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        D3D_FEATURE_LEVEL chosen{};
        check_hresult(D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
            device.GetAddressOf(), &chosen, context.GetAddressOf()));

        ComPtr<IDXGIDevice> dxgiDevice;
        check_hresult(device.As(&dxgiDevice));
        ComPtr<IDXGIAdapter> adapter;
        check_hresult(dxgiDevice->GetAdapter(adapter.GetAddressOf()));
        ComPtr<IDXGIFactory2> factory;
        check_hresult(adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf())));

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width = 1280;
        desc.Height = 720;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Stereo = FALSE;
        desc.SampleDesc.Count = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        check_hresult(factory->CreateSwapChainForCoreWindow(
            device.Get(), reinterpret_cast<IUnknown*>(get_abi(window)),
            &desc, nullptr, swapchain.GetAddressOf()));
        ComPtr<ID3D11Texture2D> backBuffer;
        check_hresult(swapchain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));
        check_hresult(device->CreateRenderTargetView(
            backBuffer.Get(), nullptr, renderTarget.GetAddressOf()));
    }
    void Run()
    {
        window.Activate();
        while (!closed)
        {
            window.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            float color[4] = { 0.035f, 0.16f, 0.30f, 1.0f }; // blue = running
            auto pads = Gamepad::Gamepads();
            if (pads.Size() > 0)
            {
                auto reading = pads.GetAt(0).GetCurrentReading();
                using B = GamepadButtons;
                const auto pressed = [&](B b) { return (reading.Buttons & b) == b; };
                if (pressed(B::A))       { color[0] = 0.01f; color[1] = 0.65f; color[2] = 0.18f; }
                else if (pressed(B::B))  { color[0] = 0.70f; color[1] = 0.06f; color[2] = 0.08f; }
                else if (pressed(B::X))  { color[0] = 0.13f; color[1] = 0.44f; color[2] = 0.80f; }
                else if (pressed(B::Y))  { color[0] = 0.82f; color[1] = 0.67f; color[2] = 0.03f; }
            }
            context->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);
            context->ClearRenderTargetView(renderTarget.Get(), color);
            check_hresult(swapchain->Present(1, 0));
        }
    }
    void Uninitialize()
    {
        renderTarget.Reset();
        swapchain.Reset();
        context.Reset();
        device.Reset();
    }
};

struct DiagnosticViewSource final : implements<DiagnosticViewSource, IFrameworkViewSource>
{
    IFrameworkView CreateView() { return make<DiagnosticView>(); }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    init_apartment(apartment_type::single_threaded);
    CoreApplication::Run(make<DiagnosticViewSource>());
    return 0;
}
