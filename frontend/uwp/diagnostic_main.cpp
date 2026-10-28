// Standalone UWP CoreApplication launch diagnostic. NOT the SoulSilver game.
// Used only to validate the Xbox One S UWP toolchain and package startup.
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/base.h>

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

struct DiagnosticView final : implements<DiagnosticView, IFrameworkView>
{
    CoreWindow window{nullptr};
    bool closed = false;

    void Initialize(CoreApplicationView const&) {}
    void SetWindow(CoreWindow const& value)
    {
        window = value;
        window.Closed([this](CoreWindow const&, CoreWindowEventArgs const&) {
            closed = true;
        });
    }
    void Load(hstring const&) {}
    void Run()
    {
        window.Activate();
        while (!closed)
            window.Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
    }
    void Uninitialize() {}
};

struct DiagnosticViewSource final : implements<DiagnosticViewSource, IFrameworkViewSource>
{
    IFrameworkView CreateView() { return make<DiagnosticView>(); }
};

int __stdcall wWinMain(void*, void*, wchar_t*, int)
{
    init_apartment();
    CoreApplication::Run(make<DiagnosticViewSource>());
    return 0;
}
