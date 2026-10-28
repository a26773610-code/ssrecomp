#include "xbox_start.h"
#include "storage.h"
#include "gamepad_reader.h"
#include "ui.h"
#include <SDL.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Storage.h>
#include <cstdio>
#include <filesystem>
#include <array>
#include <map>

namespace {
void Message(const std::string& detail)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) return;
    SDL_Window* w = SDL_CreateWindow("Pokemon DS Widescreen", 0, 0, 1280, 720, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* r = w ? SDL_CreateRenderer(w, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) : nullptr;
    if (r) {
        ssrecomp::uwp::InstallXboxBackHandler();
        Canvas c; c.Resize(1280, 720); c.Fill(0, 0, 1280, 720, 0xFF102038);
        c.Text(70, 110, "POKEMON DS WIDESCREEN", 5, 0xFFFFFFFF);
        c.Text(70, 220, detail, 3, 0xFFFFD070);
        c.Text(70, 300, "COPY YOUR ROM AS ROM.NDS TO LOCALSTATE", 3, 0xFFFFFFFF);
        c.Text(70, 350, "USING XBOX DEVICE PORTAL. THEN RESTART.", 3, 0xFFFFFFFF);
        c.Text(70, 520, "MENU: CLOSE", 3, 0xFF93A8BE);
        SDL_Texture* t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 1280, 720);
        if (t) {
            SDL_UpdateTexture(t, nullptr, c.px.data(), 1280 * 4);
            bool running = true;
            while (running) {
                SDL_Event e;
                while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
                ssrecomp::uwp::GamepadState pad;
                if (ssrecomp::uwp::ReadXboxGamepad(pad) && pad.menu) running = false;
                SDL_RenderClear(r); SDL_RenderCopy(r, t, nullptr, nullptr); SDL_RenderPresent(r);
                SDL_Delay(16);
            }
            SDL_DestroyTexture(t);
        }
        SDL_DestroyRenderer(r);
    }
    if (w) SDL_DestroyWindow(w);
    SDL_Quit();
}
}

bool XboxStart(int& argc, char**& argv)
{
    static std::vector<std::string> args;
    static std::vector<char*> pointers;
    const auto local = ssrecomp::uwp::LocalDataDirectoryUtf8();
    if (local.empty()) return false;
    const auto localPath = std::filesystem::u8path(local);
    std::filesystem::current_path(localPath);
    freopen("game.log", "w", stderr);
    setvbuf(stderr, nullptr, _IONBF, 0);
    fprintf(stderr, "[Xbox] experimental UWP: interpreter, software widescreen, native controller\n");
    std::array<char, 16> header{};
    FILE* rom = fopen("rom.nds", "rb");
    if (!rom) { Message("ROM.NDS WAS NOT FOUND"); return false; }
    const size_t got = fread(header.data(), 1, header.size(), rom); fclose(rom);
    const std::string code(header.data() + 12, 4);
    if (got != header.size()) { Message("ROM.NDS IS NOT A VALID DS ROM"); return false; }
    const std::map<std::string,std::string> names = {
        {"IPGE","soulsilver"},{"IPKE","heartgold"},{"IRBO","black"},{"IRAO","white"},
        {"IREO","black2"},{"IRDO","white2"},{"CPUE","platinum"}};
    const auto game = names.find(code);
    if (game == names.end()) { Message("THIS ROM VERSION IS NOT SUPPORTED"); return false; }
    args = {"pokemon-ds-wide", "rom.nds", "--interp", "--fullscreen", "--no-wide-gpu",
            "--save", game->second + ".sav", "--config", "pokemon-ds-wide.cfg"};
    if (code != "IPGE" && code != "IPKE") {
        const auto installed = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation().Path();
        const auto funcs = std::filesystem::path(installed.c_str()) / L"funcs" / (std::wstring(code.begin(), code.end()) + L".txt");
        if (!std::filesystem::exists(funcs)) { Message("FUNCTION TABLE IS MISSING"); return false; }
        args.push_back("--funcs"); args.push_back(funcs.string());
    }
    for (auto& arg : args) pointers.push_back(arg.data());
    argc = static_cast<int>(pointers.size()); pointers.push_back(nullptr); argv = pointers.data();
    fprintf(stderr, "[Xbox] game %.4s, save %s.sav\n", code.c_str(), game->second.c_str());
    return true;
}
