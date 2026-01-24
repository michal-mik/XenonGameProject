#include "Engine/TextureManager.hpp"

#include <iostream>

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

    const SDL_PixelFormatDetails* fmt =
        SDL_GetPixelFormatDetails(surface->format);
    if (!fmt) {
        std::cerr << "[TextureManager] SDL_GetPixelFormatDetails failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroySurface(surface);
        return nullptr;
    }

    // Magenta = transparent
    Uint32 colorkey = SDL_MapRGB(fmt, nullptr, 255, 0, 255);
    if (!SDL_SetSurfaceColorKey(surface, true, colorkey)) {
        std::cerr << "[TextureManager] SDL_SetSurfaceColorKey failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroySurface(surface);
        return nullptr;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!tex) {
        std::cerr << "[TextureManager] SDL_CreateTextureFromSurface failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroySurface(surface);
        return nullptr;
    }

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
