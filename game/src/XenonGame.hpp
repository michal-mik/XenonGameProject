#pragma once

#include "Engine/Engine.hpp"
#include "ShipPawn.hpp"
#include <SDL3/SDL.h>
#include <vector>
#include <string>

enum class GameState {
    Playing,
    BossFight,
    GameOver,
    Victory
};

class XenonGame : public IGame {
public:
    XenonGame() = default;
    ~XenonGame() override;

    bool init(const EngineContext& ctx) override;
    void handleEvent(const SDL_Event& e, bool& running) override;
    void update(float dt) override;
    void render(SDL_Renderer* renderer) override;

private:
    EngineContext m_ctx{};
    GameState m_gameState = GameState::Playing;

    // --- Ship ---
    ShipPawn m_ship;
    static constexpr int SHIP_FRAME_WIDTH  = 64;
    static constexpr int SHIP_FRAME_HEIGHT = 64;
    
    int m_weaponLevel = 0; 
    int m_lives = 3;
    int m_score = 0;
    
    // Shield
    bool m_hasShield = false;
    float m_shieldTimer = 0.0f;
    static constexpr float SHIELD_DURATION = 10.0f;

    // --- Missiles ---
    struct Missile {
        SDL_FRect rect{};
        SDL_FRect src{}; // Source rect for animation/frame selection
        float speedY = 0.0f;
        float speedX = 0.0f;
        bool alive = false;
    };
    std::vector<Missile> m_missiles;
    SDL_Texture* m_missileTexture = nullptr;
    float m_missileCooldown = 0.0f;

    // --- Enemies ---
    enum class EnemyType { Loner, Rusher };
    struct Enemy {
        EnemyType type = EnemyType::Loner;
        SDL_FRect src{};
        SDL_FRect rect{};
        float speedX = 0.0f;
        float speedY = 0.0f;
        int hp = 1;
        bool alive = false;
        float shootTimer = 0.0f;
    };
    SDL_Texture* m_lonerTexture = nullptr;
    SDL_Texture* m_rusherTexture = nullptr;
    std::vector<Enemy> m_enemies;
    float m_lonerSpawnTimer = 0.0f;
    float m_rusherSpawnTimer = 0.0f;

    // --- Enemy Projectiles ---
    struct EnemyProjectile {
        SDL_FRect rect{};
        float speedY = 0.0f;
        float speedX = 0.0f;
        bool alive = false;
    };
    SDL_Texture* m_enemyProjectileTexture = nullptr;
    std::vector<EnemyProjectile> m_enemyProjectiles;

    // --- Asteroids ---
    enum class AsteroidSize { Small, Medium, Large };
    struct Asteroid {
        AsteroidSize size;
        SDL_FRect rect;
        SDL_FRect src;
        float speedY;
        int hp;
        bool alive;
        // Animation
        int currentFrame;
        int totalFrames;
        float animTimer;
    };
    SDL_Texture* m_asteroidSTexture = nullptr;
    SDL_Texture* m_asteroidMTexture = nullptr;
    SDL_Texture* m_asteroidGTexture = nullptr;
    std::vector<Asteroid> m_asteroids;
    float m_asteroidSpawnTimer = 0.0f;

    // --- Boss ---
    struct Boss {
        SDL_FRect rect;
        int hp;
        int maxHp;
        float shootTimer;
        bool active;
        float dirX;
    } m_boss;
    SDL_Texture* m_bossTexture = nullptr;

    // --- PowerUps ---
    enum class PowerUpType { Weapon, Shield, Score, Life };
    struct PowerUp {
        PowerUpType type;
        SDL_FRect rect;
        SDL_FRect src;
        float speedY;
        bool alive;
        int currentFrame;
        int totalFrames;
        float animTimer;
    };
    SDL_Texture* m_puWeaponTexture = nullptr;
    SDL_Texture* m_puShieldTexture = nullptr;
    SDL_Texture* m_puScoreTexture = nullptr;
    SDL_Texture* m_puLifeTexture = nullptr;
    std::vector<PowerUp> m_powerups;

    // --- Explosions ---
    struct Explosion {
        SDL_FRect src;
        SDL_FRect dst;
        float frameTime;
        int currentFrame;
        int totalFrames;
        bool alive;
    };
    SDL_Texture* m_explosionTexture = nullptr;
    std::vector<Explosion> m_explosions;

    // --- Dust / Background ---
    struct DustParticle {
        SDL_Texture* texture = nullptr;
        SDL_FRect rect{};
        SDL_FRect src{}; // Added src to prevent multiplying
        float speed = 0.0f;
    };
    std::vector<DustParticle> m_dustParticles;
    SDL_Texture* m_galaxyTexture = nullptr;

    // --- Methods ---
    void fireMissile();
    void updateMissiles(float dt);
    void renderMissiles(SDL_Renderer* renderer);

    void spawnLoner();
    void spawnRusher();
    void updateEnemies(float dt);
    void renderEnemies(SDL_Renderer* renderer);

    void fireEnemyProjectile(const SDL_FRect& sourceRect, float speedY, float speedX = 0.0f);
    void updateEnemyProjectiles(float dt);
    void renderEnemyProjectiles(SDL_Renderer* renderer);

    void spawnAsteroid();
    void updateAsteroids(float dt);
    void renderAsteroids(SDL_Renderer* renderer);

    void spawnBoss();
    void updateBoss(float dt);
    void renderBoss(SDL_Renderer* renderer);

    void spawnPowerUp(float x, float y);
    void updatePowerUps(float dt);
    void renderPowerUps(SDL_Renderer* renderer);
    void applyPowerUp(PowerUpType type);

    void spawnExplosion(float cx, float cy);
    void updateExplosions(float dt);
    void renderExplosions(SDL_Renderer* renderer);

    void initDustBackground();
    void updateDust(float dt);
    void renderDust(SDL_Renderer* renderer);

    void checkCollisions();
    void onPlayerHit();
    bool rectsOverlap(const SDL_FRect& a, const SDL_FRect& b);

    // HUD
    SDL_Texture* m_fontTexture = nullptr;
    SDL_Texture* m_lifeIconTexture = nullptr;
    void drawText(SDL_Renderer* r, float x, float y, const std::string& text);
    void renderHUD(SDL_Renderer* r);

    void renderText(SDL_Renderer* renderer, const std::string& text, float x, float y);
    bool m_gameOver = false;
};