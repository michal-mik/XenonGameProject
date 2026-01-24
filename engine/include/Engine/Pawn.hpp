#pragma once

#include "Engine/Actor.hpp"

class Pawn : public Actor {
public:
    Pawn() = default;
    ~Pawn() override = default;

    void setVelocity(float vx, float vy) { m_velocity.x = vx; m_velocity.y = vy; }
    SDL_FPoint getVelocity() const { return m_velocity; }

    // default behaviour: move according to velocity
    void update(float dt) override {
        m_rect.x += m_velocity.x * dt;
        m_rect.y += m_velocity.y * dt;
    }

protected:
    SDL_FPoint m_velocity{0.0f, 0.0f};
};
