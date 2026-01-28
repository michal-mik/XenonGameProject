#include "Engine/TextureManager.hpp"
#include <iostream>
#include <SDL3/SDL.h> // Ensure SDL is included

TextureManager::~TextureManager()
{
    clear();
}

void TextureManager::setRenderer(SDL_Renderer* renderer)
{
    m_renderer = renderer;
}

SDL_Texture* TextureManager::load(const std::string& path)
{
    if (!m_renderer) {
        std::cerr << "[TextureManager] Renderer not set\n";
        return nullptr;
    }

    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        return it->second;
    }

    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::cerr << "[TextureManager] SDL_LoadBMP failed for " << path
                  << ": " << SDL_GetError() << "\n";
        return nullptr;
    }

    // Get pixel format details
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surface->format);
    if (!fmt) {
        std::cerr << "[TextureManager] SDL_GetPixelFormatDetails failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroySurface(surface);
        return nullptr;
    }

    // FIX: Pass the surface's palette. If it's an 8-bit BMP, MapRGB needs the palette to find the index.
    // Magenta (255, 0, 255) = Transparent
    Uint32 colorkey = SDL_MapRGB(fmt, SDL_GetSurfacePalette(surface), 255, 0, 255);
    
    if (!SDL_SetSurfaceColorKey(surface, true, colorkey)) {
        // Not always a fatal error, but good to log
        // std::cerr << "[TextureManager] Warning: SDL_SetSurfaceColorKey failed or not needed: " << SDL_GetError() << "\n";
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!tex) {
        std::cerr << "[TextureManager] SDL_CreateTextureFromSurface failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroySurface(surface);
        return nullptr;
    }

    // Optional: Set texture blend mode to handle alpha blending if the bmp had alpha
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_DestroySurface(surface);

    m_cache[path] = tex;
    return tex;
}

void TextureManager::clear()
{
    for (auto& [path, tex] : m_cache) {
        (void)path;
        if (tex) {
            SDL_DestroyTexture(tex);
        }
    }
    m_cache.clear();
}