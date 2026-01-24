#pragma once

#include "Engine/Engine.hpp"
#include "ShipPawn.hpp"

#include <SDL3/SDL.h>

#include <vector>
#include <algorithm>

class XenonGame : public IGame {
public:
    XenonGame() = default;
    ~XenonGame() override;

    bool init(const EngineContext& ctx) override;
    void handleEvent(const SDL_Event& e, bool& running) override;
    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

private:
    EngineContext m_ctx{};  // window, renderer, world, size

    // --- Ship ----------------------------------------------------
    ShipPawn m_ship;

    static constexpr int SHIP_FRAME_WIDTH  = 64;
    static constexpr int SHIP_FRAME_HEIGHT = 64;

    // --- Missiles -----------------------------------------------
    struct Missile {
        SDL_FRect rect{};   // position + size
        float     speedY = 0.0f;
        bool      alive  = false;
    };

    std::vector<Missile> m_missiles;


    SDL_Texture*        m_missileTexture = nullptr;
    float               m_missileCooldown = 0.0f;
    static constexpr float MISSILE_COOLDOWN = 0.15f; // seconds

    void fireMissile();
    void updateMissiles(float dt);
    void renderMissiles(SDL_Renderer* renderer);

    // --- Enemies (Loners + Rushers) --------------------------------
    enum class EnemyType { Loner, Rusher };

    struct Enemy {
        EnemyType type = EnemyType::Loner;
        SDL_FRect src{};
        SDL_FRect rect{};
        float     speedX   = 0.0f;
        float     speedY   = 0.0f;
        bool      alive    = false;
        float     shootTimer = 0.0f; // for loner bullets
    };

    SDL_Texture*       m_lonerTexture    = nullptr;
    SDL_Texture*       m_rusherTexture   = nullptr;
    std::vector<Enemy> m_enemies;

    float m_lonerSpawnTimer  = 0.0f;
    float m_rusherSpawnTimer = 0.0f;

    static constexpr float LONER_SPAWN_INTERVAL   = 2.0f; // seconds
    static constexpr float RUSHER_SPAWN_INTERVAL  = 5.0f; // seconds
    static constexpr int   LONER_FRAME_WIDTH      = 64;
    static constexpr int   LONER_FRAME_HEIGHT     = 64;
    static constexpr int   RUSHER_FRAME_WIDTH     = 64;
    static constexpr int   RUSHER_FRAME_HEIGHT    = 64;

    void spawnLoner();
    void spawnRusher();
    void updateEnemies(float dt);
    void renderEnemies(SDL_Renderer* renderer);

    // --- Enemy projectiles ----------------------------------------
    struct EnemyProjectile {
        SDL_FRect rect{};
        float     speedY = 0.0f;
        bool      alive  = false;
    };

    SDL_Texture*                 m_enemyProjectileTexture = nullptr;
    std::vector<EnemyProjectile> m_enemyProjectiles;

    static constexpr float ENEMY_PROJECTILE_SPEED = 200.0f;

    void fireEnemyProjectile(const Enemy& from);
    void updateEnemyProjectiles(float dt);
    void renderEnemyProjectiles(SDL_Renderer* renderer);

    // collisions
    static bool rectsOverlap(const SDL_FRect& a, const SDL_FRect& b);
    void handleMissileEnemyCollisions();
    void handleEnemyProjectileShipCollisions();

    // --- Explosions ---------------------------------------------
    struct Explosion {
        SDL_FRect src;
        SDL_FRect dst;
        float frameTime;
        int   currentFrame;
        int   totalFrames;
        bool  alive;
    };

    SDL_Texture*          m_explosionTexture = nullptr;
    std::vector<Explosion> m_explosions;

    static constexpr int   EXPLOSION_FRAME_SIZE   = 64;  // explode64.bmp tile
    static constexpr int   EXPLOSION_COLUMNS      = 8;   // 8 frames per row
    static constexpr int   EXPLOSION_TOTAL_FRAMES = EXPLOSION_COLUMNS;
    static constexpr float EXPLOSION_FPS          = 30.0f;

    void spawnExplosion(float cx, float cy);
    void updateExplosions(float dt);
    void renderExplosions(SDL_Renderer* renderer);

    // --- gamepad state -------------------------------------------------
    SDL_Gamepad* m_gamepad         = nullptr;
    bool         m_gamepadConnected = false;
    float        m_gamepadDeadzone  = 0.25f; // normalized deadzone


    // --- Background dust (level) --------------------------------------
    struct DustParticle {
        SDL_Texture* texture = nullptr;
        SDL_FRect    rect{};
        float        speed = 0.0f;
    };

    std::vector<DustParticle> m_dustParticles;

    void initDustBackground();
    void updateDust(float dt);
    void renderDust(SDL_Renderer* renderer);

    // --- Level / score --------------------------------------------
    struct LevelConfig {
        int   killTarget;
        float lonerInterval;
        float rusherInterval;
    };

    int         m_currentLevel     = 0;
    int         m_killsInLevel     = 0;
    int         m_score            = 0;
    int         m_lives            = 3;
    LevelConfig m_levelConfig{10, 2.0f, 5.0f};

    void setupLevel(int index);
    void onEnemyKilled();
};
