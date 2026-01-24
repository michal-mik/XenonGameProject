#pragma once

#include "Engine/Pawn.hpp"
#include "Engine/TextureManager.hpp"

class ShipPawn : public Pawn {
public:
    ShipPawn() = default;
    ~ShipPawn() override = default;

    bool init(TextureManager* textures,
              const char* spritePath,
              int frameWidth,
              int frameHeight,
              int windowWidth,
              int windowHeight);

    void setSpeed(float speed) { m_speed = speed; }

    // called by XenonGame when input changes
    void setMoveLeft(bool v)  { m_moveLeft  = v; }
    void setMoveRight(bool v) { m_moveRight = v; }
    void setMoveUp(bool v)    { m_moveUp    = v; }
    void setMoveDown(bool v)  { m_moveDown  = v; }

    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

private:
    TextureManager* m_textures = nullptr;
    SDL_Texture*    m_texture  = nullptr;

    int   m_frameWidth  = 0;
    int   m_frameHeight = 0;
    int   m_currentFrame = 3;   // left=1, idle=3, right=5

    float m_speed = 300.0f;

    bool m_moveLeft  = false;
    bool m_moveRight = false;
    bool m_moveUp    = false;
    bool m_moveDown  = false;
};
