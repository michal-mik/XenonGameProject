#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

class TextureManager {
public:
    TextureManager() = default;
    ~TextureManager();

    // Must be called once after the renderer is created
    void setRenderer(SDL_Renderer* renderer);

    // Load or fetch from cache
    SDL_Texture* load(const std::string& path);

    // Destroy all cached textures (called by Engine on shutdown)
    void clear();

private:
    SDL_Renderer* m_renderer = nullptr;
    std::unordered_map<std::string, SDL_Texture*> m_cache;
};
