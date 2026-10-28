#include <windows.h>
#include <SDL_main.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return SDL_WinRTRunApp(SDL_main, nullptr);
}
