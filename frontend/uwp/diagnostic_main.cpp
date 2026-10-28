// Xbox One S UWP launch diagnostic v0.4. This is NOT Pokemon SoulSilver.
// Writes the startup stage and any failing HRESULT to LocalState/ssrecomp-diagnostic.log.
#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

using namespace winrt;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::Gaming::Input;
using Microsoft::WRL::ComPtr;

namespace {
void Log(char const* stage, HRESULT hr = S_OK) noexcept
{
    char line[320]{};
    std::snprintf(line, sizeof(line), "Diagnostic v0.4 | %s | HRESULT=0x%08lX\r\n",
                  stage, static_cast<unsigned long>(hr));
    OutputDebugStringA(line);
    try
    {
        std::wstring path = winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path().c_str();
        path += L"\\ssrecomp-diagnostic.log";
        HANDLE h = CreateFile2(path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ,
                               OPEN_ALWAYS, nullptr);
        if (h != INVALID_HANDLE_VALUE)
        {
            DWORD written{};
            WriteFile(h, line, static_cast<DWORD>(std::strlen(line)), &written, nullptr);
            CloseHandle(h);
        }
    }
    catch (...) { /* Logging itself must not crash the app. */ }
}
bool Ok(HRESULT hr, char const* step) noexcept
{
    Log(step, hr);
    return SUCCEEDED(hr);
}
}

struct DiagnosticView : implements<DiagnosticView, IFrameworkView>
{
    CoreWindow window{nullptr};
    bool closed = false;
    bool graphicsReady = false;
    bool gamepadEnabled = true;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGISwapChain1> swapchain;
    ComPtr<ID3D11RenderTargetView> target;

    void Initialize(CoreApplicationView const&)
    {
        Log("Initialize");
    }
    void SetWindow(CoreWindow const& value)
    {
        window = value;
        Log("SetWindow");
        window.Closed([this](auto const&, auto const&) {
            closed = true;
            Log("CoreWindow closed");
        });
    }
    void Load(hstring const&)
    {
        // Never touch Direct3D before Run activates the CoreWindow.
        Log("Load (graphics deferred)");
    }
    bool InitGraphics()
    {
        Log("InitGraphics begin");
        const D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0
        };
        D3D_FEATURE_LEVEL chosen{};
        if (!Ok(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, ARRAYSIZE(levels),
                D3D11_SDK_VERSION, device.GetAddressOf(), &chosen,
                context.GetAddressOf()), "D3D11CreateDevice")) return false;

        ComPtr<IDXGIDevice> dxgiDevice;
        if (!Ok(device.As(&dxgiDevice), "Query IDXGIDevice")) return false;
        ComPtr<IDXGIAdapter> adapter;
        if (!Ok(dxgiDevice->GetAdapter(adapter.GetAddressOf()), "GetAdapter")) return false;
        ComPtr<IDXGIFactory2> factory;
        if (!Ok(adapter->GetParent(IID_PPV_ARGS(factory.GetAddressOf())),
                "GetParent IDXGIFactory2")) return false;

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width = 1280;
        desc.Height = 720;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        if (!Ok(factory->CreateSwapChainForCoreWindow(device.Get(),
                reinterpret_cast<IUnknown*>(get_abi(window)), &desc,
                nullptr, swapchain.GetAddressOf()),
                "CreateSwapChainForCoreWindow")) return false;

        ComPtr<ID3D11Texture2D> buffer;
        if (!Ok(swapchain->GetBuffer(0, IID_PPV_ARGS(buffer.GetAddressOf())),
                "GetBuffer")) return false;
        if (!Ok(device->CreateRenderTargetView(buffer.Get(), nullptr,
                target.GetAddressOf()), "CreateRenderTargetView")) return false;
        Log("Graphics ready (blue screen expected)");
        return true;
    }
    void Run()
    {
        Log("Run start");
        // On Xbox, controller B also produces the system BackRequested event.
        // If unhandled, the shell returns to DevHome even though input polling works.
        // Handle it here so B can be reserved for the Nintendo DS cancel action.
        auto navigation = SystemNavigationManager::GetForCurrentView();
        auto backToken = navigation.BackRequested([this](auto const&, BackRequestedEventArgs const& args) {
            args.Handled(true);
            Log("Xbox B / BackRequested handled (stay in application)");
        });
        try
        {
            window.Activate();
            Log("CoreWindow activated");
            window.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            graphicsReady = InitGraphics();
        }
        catch (winrt::hresult_error const& e)
        {
            Log("Initial activation or graphics exception", e.code());
        }
        if (!graphicsReady) Log("Graphics unavailable, keeping window open");

        while (!closed)
        {
            window.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            if (!graphicsReady) { Sleep(100); continue; }
            float color[4] = {0.035f, 0.16f, 0.30f, 1.0f};
            if (gamepadEnabled)
            {
                try
                {
                    auto pads = Gamepad::Gamepads();
                    if (pads.Size() > 0)
                    {
                        auto reading = pads.GetAt(0).GetCurrentReading();
                        using B = GamepadButtons;
                        auto pressed = [&](B b) { return (reading.Buttons & b) == b; };
                        if (pressed(B::A))      { color[0]=0.01f; color[1]=0.65f; color[2]=0.18f; }
                        else if (pressed(B::B)) { color[0]=0.70f; color[1]=0.06f; color[2]=0.08f; }
                        else if (pressed(B::X)) { color[0]=0.13f; color[1]=0.44f; color[2]=0.80f; }
                        else if (pressed(B::Y)) { color[0]=0.82f; color[1]=0.67f; color[2]=0.03f; }
                    }
                }
                catch (winrt::hresult_error const& e)
                {
                    Log("Gamepad read failed; disabling input", e.code());
                    gamepadEnabled = false;
                }
            }
            context->OMSetRenderTargets(1, target.GetAddressOf(), nullptr);
            context->ClearRenderTargetView(target.Get(), color);
            HRESULT hr = swapchain->Present(1, 0);
            if (FAILED(hr))
            {
                Log("Present failed; keeping window open", hr);
                graphicsReady = false;
            }
        }
        navigation.BackRequested(backToken);
        Log("Run ended (window closed)");
    }
    void Uninitialize()
    {
        Log("Uninitialize");
        target.Reset(); swapchain.Reset(); context.Reset(); device.Reset();
    }
};

struct DiagnosticViewSource : implements<DiagnosticViewSource, IFrameworkViewSource>
{
    IFrameworkView CreateView() { return make<DiagnosticView>(); }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    try
    {
        // CoreApplication manages the UWP application thread's COM apartment.
        // Explicit single-threaded RoInitialize on an already-initialized MTA
        // produced RPC_E_CHANGED_MODE (0x80010106) on the Xbox One S.
        // Follow Microsoft's C++/WinRT Core App example: call Run directly.
        Log("wWinMain entered (COM apartment managed by CoreApplication)");
        CoreApplication::Run(make<DiagnosticViewSource>());
        Log("CoreApplication::Run returned");
        return 0;
    }
    catch (winrt::hresult_error const& e)
    {
        Log("WinRT exception escaped CoreApplication", e.code());
        return 1;
    }
    catch (...)
    {
        Log("Unknown exception escaped CoreApplication", E_FAIL);
        return 2;
    }
}
