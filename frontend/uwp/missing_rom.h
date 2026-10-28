#pragma once
// Xbox UWP: visible setup screen before a user provides their own game ROM.
// This UI contains no game assets, ROM bytes, or embedded copyrighted images.
// Requires the already-initialized SDL2/WinRT entry point from this project.
#include <SDL.h>
#include <filesystem>
#include <system_error>
#include <string>
#include <cctype>
#include <cstdio>
#include "gamepad_reader.h"

namespace ssrecomp::uwp {
namespace bootui {
// Five-column, seven-row pixel font, kept tiny and self-contained for Xbox.
inline const char* Glyph(char c)
{
    switch (c) {
    case 'A': return "01110100011000111111100011000110001";
    case 'B': return "11110100011000111110100011000111110";
    case 'C': return "01111100001000010000100001000001111";
    case 'D': return "11110100011000110001100011000111110";
    case 'E': return "11111100001000011110100001000011111";
    case 'F': return "11111100001000011110100001000010000";
    case 'G': return "01111100001000010111100011000101111";
    case 'H': return "10001100011000111111100011000110001";
    case 'I': return "11111001000010000100001000010011111";
    case 'J': return "00111000100001000010100101001001100";
    case 'K': return "10001100101010011000101001001010001";
    case 'L': return "10000100001000010000100001000011111";
    case 'M': return "10001110111010110101100011000110001";
    case 'N': return "10001110011010110011100011000110001";
    case 'O': return "01110100011000110001100011000101110";
    case 'P': return "11110100011000111110100001000010000";
    case 'Q': return "01110100011000110001101011001001101";
    case 'R': return "11110100011000111110101001001010001";
    case 'S': return "01111100001000001110000010000111110";
    case 'T': return "11111001000010000100001000010000100";
    case 'U': return "10001100011000110001100011000101110";
    case 'V': return "10001100011000110001100010101000100";
    case 'W': return "10001100011000110101101011101110001";
    case 'X': return "10001100010101000100010101000110001";
    case 'Y': return "10001100010101000100001000010000100";
    case 'Z': return "11111000010001000100010001000011111";
    case '0': return "01110100011001110101110011000101110";
    case '1': return "00100011000010000100001000010001110";
    case '2': return "01110100010000100010001000100011111";
    case '3': return "11110000010000101110000010000111110";
    case '4': return "00010001100101010010111110001000010";
    case '5': return "11111100001000011110000010000111110";
    case '6': return "01111100001000011110100011000101110";
    case '7': return "11111000010001000100010000100001000";
    case '8': return "01110100011000101110100011000101110";
    case '9': return "01110100011000101111000010000111110";
    case '.': return "00000000000000000000000000011000110";
    case '/': return "00001000010001000100010001000000000";
    case '-': return "00000000000000011111000000000000000";
    case ':': return "00000011000110000000011000110000000";
    case '_': return "00000000000000000000000000000011111";
    default: return "00000000000000000000000000000000000";
    }
}

inline void DrawText(SDL_Renderer* renderer, std::string label, int x, int y, int size)
{
    for (const unsigned char raw : label) {
        const char* bits = Glyph(static_cast<char>(std::toupper(raw)));
        for (int row=0; row<7; ++row)
            for (int col=0; col<5; ++col)
                if (bits[row*5+col] == '1') {
                    SDL_Rect dot{x+col*size, y+row*size, size-1, size-1};
                    SDL_RenderFillRect(renderer, &dot);
                }
        x += 6*size;
    }
}
}

inline bool WaitForPrivateRom(const std::string& romPath)
{
    std::error_code ioError;
    if (std::filesystem::exists(romPath, ioError)) return true;
    fprintf(stderr, "[Xbox UWP] Missing user's ROM. Waiting in single-screen setup view: %s\n",
            romPath.c_str());
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "[Xbox UWP] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_Window* window = SDL_CreateWindow("SSRecomp - Install your own ROM",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!window) {
        fprintf(stderr, "[Xbox UWP] Setup window failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    // The B button is a Windows system Back command unless intercepted.
    InstallXboxBackHandler();
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        fprintf(stderr, "[Xbox UWP] Setup renderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    SDL_RenderSetLogicalSize(renderer, 1280, 720);
    bool running = true;
    Uint32 lastCheck = 0;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        if (SDL_GetTicks() - lastCheck > 1000) {
            lastCheck = SDL_GetTicks();
            ioError.clear();
            if (std::filesystem::exists(romPath, ioError)) {
                fprintf(stderr, "[Xbox UWP] ROM detected; entering interpreter beta\n");
                break;
            }
        }
        SDL_SetRenderDrawColor(renderer, 13, 26, 48, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 77, 154, 250, 255);
        SDL_Rect bar{82, 113, 14, 410};
        SDL_RenderFillRect(renderer, &bar);
        SDL_SetRenderDrawColor(renderer, 236, 244, 255, 255);
        bootui::DrawText(renderer, "SOULSILVER", 120, 115, 8);
        SDL_SetRenderDrawColor(renderer, 245, 184, 94, 255);
        bootui::DrawText(renderer, "ROM NOT FOUND", 120, 240, 6);
        SDL_SetRenderDrawColor(renderer, 208, 222, 245, 255);
        bootui::DrawText(renderer, "OPEN XBOX DEVICE PORTAL", 120, 345, 4);
        bootui::DrawText(renderer, "COPY ROM.NDS TO LOCALSTATE", 120, 400, 4);
        SDL_SetRenderDrawColor(renderer, 134, 171, 204, 255);
        bootui::DrawText(renderer, "AUTOMATIC START AFTER COPY", 120, 485, 4);
        bootui::DrawText(renderer, "TEST VERSION - INTERPRETER", 120, 575, 3);
        SDL_RenderPresent(renderer);
        SDL_Delay(30);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    if (!running) return false;
    ioError.clear();
    return std::filesystem::exists(romPath, ioError);
}
} // namespace ssrecomp::uwp
