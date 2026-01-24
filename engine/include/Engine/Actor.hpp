#pragma once

#include <SDL3/SDL.h>

class Actor {
public:
    Actor() = default;
    virtual ~Actor() = default;

    // default no-op; subclasses can override
    virtual void update(float /*dt*/) {}

    // must be implemented by subclasses
    virtual void render(SDL_Renderer* renderer) = 0;

    void setPosition(float x, float y) { m_rect.x = x; m_rect.y = y; }
    void setSize(float w, float h)     { m_rect.w = w; m_rect.h = h; }

    SDL_FRect getRect() const { return m_rect; }

    bool isAlive() const { return m_alive; }
    void kill()          { m_alive = false; }

protected:
    SDL_FRect m_rect{0.f, 0.f, 0.f, 0.f};
    bool      m_alive = true;
};
