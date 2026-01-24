#include "ShipPawn.hpp"
#include <iostream>
#include <string>

bool ShipPawn::init(TextureManager* textures,
                    const char* spriteName,
                    int frameWidth,
                    int frameHeight,
                    int windowWidth,
                    int windowHeight)
{
    m_textures    = textures;
    m_frameWidth  = frameWidth;
    m_frameHeight = frameHeight;

    if (!m_textures) {
        std::cerr << "[ShipPawn] No TextureManager\n";
        return false;
    }

    // Always load from graphics/ folder
    std::string fullPath = "graphics/";
    fullPath += spriteName;

    std::cout << "[ShipPawn] loading sprite: " << fullPath << "\n";

    m_texture = m_textures->load(fullPath);
    
    if (!m_texture) {
        std::cerr << "[ShipPawn] Failed to load sprite: " << fullPath << "\n";
        return false;
    }

    // position at bottom center
    m_rect.w = static_cast<float>(m_frameWidth);
    m_rect.h = static_cast<float>(m_frameHeight);
    m_rect.x = (windowWidth  - m_frameWidth)  * 0.5f;
    m_rect.y = (windowHeight - m_frameHeight) - 20.0f;

    m_currentFrame = 3; // idle

    return true;
}


void ShipPawn::update(float dt)
{
    float vx = 0.0f;
    float vy = 0.0f;

    if (m_moveLeft)  vx -= m_speed;
    if (m_moveRight) vx += m_speed;
    if (m_moveUp)    vy -= m_speed;
    if (m_moveDown)  vy += m_speed;

    setVelocity(vx, vy);

    // move using Pawn's default movement
    Pawn::update(dt);

    // pick animation frame based on horizontal movement
    const float eps = 5.0f;
    if (vx < -eps)      m_currentFrame = 1;
    else if (vx > eps)  m_currentFrame = 5;
    else                m_currentFrame = 3;
}

void ShipPawn::render(SDL_Renderer* renderer)
{
    if (!m_texture) return;

    SDL_FRect src;
    src.w = static_cast<float>(m_frameWidth);
    src.h = static_cast<float>(m_frameHeight);
    src.x = static_cast<float>(m_currentFrame * m_frameWidth);
    src.y = 0.0f;

    SDL_RenderTexture(renderer, m_texture, &src, &m_rect);
}
