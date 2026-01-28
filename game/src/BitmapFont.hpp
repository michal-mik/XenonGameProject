#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <cctype>

class BitmapFont {
public:
    // Xenon 2000 Font8x8 usually has characters in a grid
    // We assume standard ASCII layout or similar.
    // 8x8 pixels per char.
    void init(SDL_Texture* texture) {
        m_texture = texture;
    }

    void draw(SDL_Renderer* renderer, float x, float y, const std::string& text) {
        if (!m_texture) return;

        SDL_FRect src;
        src.w = 8.0f;
        src.h = 8.0f;

        SDL_FRect dst;
        dst.w = 8.0f;
        dst.h = 8.0f;
        dst.x = x;
        dst.y = y;

        for (char c : text) {
            // Mapping ASCII to grid (assuming 16 columns in Font8x8.bmp)
            int ascii = static_cast<unsigned char>(c);
            int col = ascii % 16;
            int row = ascii / 16;

            src.x = static_cast<float>(col * 8);
            src.y = static_cast<float>(row * 8);

            SDL_RenderTexture(renderer, m_texture, &src, &dst);
            dst.x += 8.0f; // Advance cursor
        }
    }

private:
    SDL_Texture* m_texture = nullptr;
};