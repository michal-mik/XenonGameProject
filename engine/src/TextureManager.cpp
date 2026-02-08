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
    // Check if renderer is set
    if (!m_renderer) {
        std::cerr << "[TextureManager] Renderer not set\n";
        return nullptr;
    }

    // Check cache first
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        return it->second;
    }

    // Load BMP surface
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface) {
        std::cerr << "[TextureManager] SDL_LoadBMP failed for " << path
                  << ": " << SDL_GetError() << "\n";
        return nullptr;
    }

    // --- FIX STARTS HERE ---
    
    // In SDL3, surface->format is an enum (Uint32), not a struct pointer.
    // We must retrieve the format details to use SDL_MapRGB.
    const SDL_PixelFormatDetails* fmt = SDL_GetPixelFormatDetails(surface->format);
    
    if (fmt) {
        // We also need the palette if the image is 8-bit (indexed color).
        // Standard SDL_MapRGB needs the palette pointer to match colors correctly.
        SDL_Palette* palette = SDL_GetSurfacePalette(surface);

        // Map Magenta (R:255, G:0, B:255) to a pixel value
        Uint32 colorkey = SDL_MapRGB(fmt, palette, 255, 0, 255);

        // Set the color key on the surface
        if (!SDL_SetSurfaceColorKey(surface, true, colorkey)) {
             std::cerr << "[TextureManager] Warning: SDL_SetSurfaceColorKey failed: " 
                       << SDL_GetError() << "\n";
        }
    } else {
        std::cerr << "[TextureManager] Failed to get pixel format details: " 
                  << SDL_GetError() << "\n";
    }
    
    // --- FIX ENDS HERE ---

    // Create texture from the surface (now with color key set)
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (!tex) {
        std::cerr << "[TextureManager] SDL_CreateTextureFromSurface failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroySurface(surface);
        return nullptr;
    }

    // Scale mode: Nearest pixel sampling to keep pixel art sharp
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

    // Clean up surface
    SDL_DestroySurface(surface);

    // Store in cache
    m_cache[path] = tex;

    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    return tex;
}

void TextureManager::clear()
{
    for (auto& [path, tex] : m_cache) {
        (void)path; // unused warning silencer
        if (tex) {
            SDL_DestroyTexture(tex);
        }
    }
    m_cache.clear();
}