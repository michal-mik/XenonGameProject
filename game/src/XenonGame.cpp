#include "XenonGame.hpp"
#include "ShipPawn.hpp"
#include "Engine/TextureManager.hpp" 

#include <iostream>
#include <random>
#include <string>
#include <algorithm>
#include <cmath>

namespace {
    constexpr float MISSILE_WIDTH  = 8.0f;
    constexpr float MISSILE_HEIGHT = 16.0f;
    constexpr float MISSILE_SPEED  = 500.0f;
    
    std::mt19937 rng{std::random_device{}()};
    float randomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }
}

XenonGame::~XenonGame() {}

bool XenonGame::init(const EngineContext& ctx)
{
    m_ctx = ctx;
    m_fontTexture = m_ctx.textures->load("graphics/Font8x8.bmp");

    if (!m_ctx.textures) return false;

    // --- Load Textures ---
    if (!m_ship.init(m_ctx.textures, "Ship1.bmp", SHIP_FRAME_WIDTH, SHIP_FRAME_HEIGHT, m_ctx.width, m_ctx.height)) return false;
    m_ship.setSpeed(350.0f);

    // Projectiles & Effects
    m_missileTexture         = m_ctx.textures->load("graphics/missile.bmp");
    m_enemyProjectileTexture = m_ctx.textures->load("graphics/EnWeap6.bmp");
    m_explosionTexture       = m_ctx.textures->load("graphics/explode64.bmp");

    // Enemies
    m_lonerTexture  = m_ctx.textures->load("graphics/LonerA.bmp");
    m_rusherTexture = m_ctx.textures->load("graphics/rusher.bmp");
    m_bossTexture   = m_ctx.textures->load("graphics/bosseyes2.bmp");

    // Asteroids
    m_asteroidSTexture = m_ctx.textures->load("graphics/SAster64.bmp");
    m_asteroidMTexture = m_ctx.textures->load("graphics/MAster64.bmp"); 
    m_asteroidGTexture = m_ctx.textures->load("graphics/GAster96.bmp");

    // PowerUps
    m_puWeaponTexture = m_ctx.textures->load("graphics/PUWeapon.bmp");
    m_puShieldTexture = m_ctx.textures->load("graphics/PUShield.bmp");
    m_puScoreTexture  = m_ctx.textures->load("graphics/PUScore.bmp");
    m_puLifeTexture   = m_ctx.textures->load("graphics/PULife.bmp");

    // UI & Background
    m_fontTexture     = m_ctx.textures->load("graphics/Font8x8.bmp");
    m_lifeIconTexture = m_ctx.textures->load("graphics/PULife.bmp"); 
    m_galaxyTexture   = m_ctx.textures->load("graphics/galaxy2.bmp");

    if (m_fontTexture) SDL_SetTextureBlendMode(m_fontTexture, SDL_BLENDMODE_BLEND);

    // Timers
    m_lonerSpawnTimer  = 1.0f;
    m_rusherSpawnTimer = 3.0f;
    m_asteroidSpawnTimer = 2.0f;

    initDustBackground();

    return true;
}

void XenonGame::handleEvent(const SDL_Event& e, bool& running)
{
    if (m_gameState == GameState::GameOver && e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_R) {
        // Reset Game
        m_gameState = GameState::Playing;
        m_lives = 3;
        m_score = 0;
        m_weaponLevel = 0;
        m_hasShield = false;
        m_enemies.clear();
        m_asteroids.clear();
        m_missiles.clear();
        m_enemyProjectiles.clear();
        m_powerups.clear();
        m_ship.setPosition(m_ctx.width/2.0f - 32.0f, m_ctx.height - 100.0f);
        m_ship.kill(); // Resurrect
        return;
    }

    switch (e.type) {
    case SDL_EVENT_KEY_DOWN:
        if (e.key.repeat) break;
        switch (e.key.key) {
        case SDLK_ESCAPE: running = false; break;
        case SDLK_LEFT:  case SDLK_A: m_ship.setMoveLeft(true); break;
        case SDLK_RIGHT: case SDLK_D: m_ship.setMoveRight(true); break;
        case SDLK_UP:    case SDLK_W: m_ship.setMoveUp(true); break;
        case SDLK_DOWN:  case SDLK_S: m_ship.setMoveDown(true); break;
        case SDLK_SPACE: fireMissile(); break;
        }
        break;
    case SDL_EVENT_KEY_UP:
        switch (e.key.key) {
        case SDLK_LEFT:  case SDLK_A: m_ship.setMoveLeft(false); break;
        case SDLK_RIGHT: case SDLK_D: m_ship.setMoveRight(false); break;
        case SDLK_UP:    case SDLK_W: m_ship.setMoveUp(false); break;
        case SDLK_DOWN:  case SDLK_S: m_ship.setMoveDown(false); break;
        }
        break;
    }
}

void XenonGame::update(float dt)
{
    updateDust(dt);
    updateExplosions(dt);

    if (m_gameState == GameState::GameOver || m_gameState == GameState::Victory) return;

    if (m_hasShield) {
        m_shieldTimer -= dt;
        if (m_shieldTimer <= 0.0f) m_hasShield = false;
    }

    m_ship.update(dt);
    
    // Clamp ship
    SDL_FRect r = m_ship.getRect();
    if (r.x < 0) r.x = 0;
    if (r.x + r.w > m_ctx.width) r.x = m_ctx.width - r.w;
    if (r.y < 0) r.y = 0;
    if (r.y + r.h > m_ctx.height) r.y = m_ctx.height - r.h;
    m_ship.setPosition(r.x, r.y);

    if (m_missileCooldown > 0.0f) m_missileCooldown -= dt;

    // Spawning logic
    if (m_gameState == GameState::Playing) {
        m_lonerSpawnTimer -= dt;
        if (m_lonerSpawnTimer <= 0) {
            spawnLoner();
            m_lonerSpawnTimer = 2.0f;
        }

        m_rusherSpawnTimer -= dt;
        if (m_rusherSpawnTimer <= 0) {
            spawnRusher();
            m_rusherSpawnTimer = 4.0f;
        }

        m_asteroidSpawnTimer -= dt;
        if (m_asteroidSpawnTimer <= 0) {
            spawnAsteroid();
            m_asteroidSpawnTimer = randomFloat(1.5f, 3.5f);
        }

        // Boss trigger (Score based or just random for demo)
        if (m_score > 2000) { 
            m_gameState = GameState::BossFight;
            spawnBoss();
        }
    }
    else if (m_gameState == GameState::BossFight) {
        updateBoss(dt);
    }

    updateMissiles(dt);
    updateEnemies(dt);
    updateAsteroids(dt);
    updateEnemyProjectiles(dt);
    updatePowerUps(dt);

    checkCollisions();
}

void XenonGame::render(SDL_Renderer* r)
{
    if (m_galaxyTexture) SDL_RenderTexture(r, m_galaxyTexture, nullptr, nullptr);
    renderDust(r);

    renderAsteroids(r);
    renderPowerUps(r);
    renderEnemies(r);
    if (m_gameState == GameState::BossFight) renderBoss(r);
    
    m_ship.render(r);

    if (m_hasShield) {
        SDL_SetRenderDrawColor(r, 0, 200, 255, 100);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_FRect sr = m_ship.getRect();
        sr.x -= 5; sr.y -= 5; sr.w += 10; sr.h += 10;
        SDL_RenderFillRect(r, &sr);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

    renderMissiles(r);
    renderEnemyProjectiles(r);
    renderExplosions(r);
    renderHUD(r);

    renderDust(r);
    if (!m_gameOver) m_ship.render(r);
    renderEnemies(r);
    renderEnemyProjectiles(r);
    renderMissiles(r);
    renderExplosions(r);

    std::string scoreStr = "SCORE: " + std::to_string(m_score);
    renderText(r, scoreStr, 10.0f, 10.0f);

    if (m_gameOver) {
        std::string overMsg = "GAME OVER";
        // Jednoduché vycentrovanie (ak je okno 640 široké)
        float cx = (m_ctx.width / 2.0f) - ((overMsg.length() * 8.0f) / 2.0f);
        float cy = (m_ctx.height / 2.0f) - 4.0f;
        renderText(r, overMsg, cx, cy);
    }    
}

// Helper to render text using a font texture (independent of XenonGame class)
static void renderTextHelper(SDL_Renderer* renderer, SDL_Texture* fontTexture, const std::string& text, float x, float y, float scale) {
    if (!fontTexture) return;

    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        // Standard ASCII offset: space is index 32
        int charIdx = static_cast<int>(c) - 32;
        if (charIdx < 0) charIdx = 0;

        // Assume font sheet is 16xN characters of 8x8 tiles
        SDL_FRect src = { static_cast<float>((charIdx % 16) * 8), static_cast<float>((charIdx / 16) * 8), 8.0f, 8.0f };
        
        // Round coordinates to avoid sub-pixel blurring
        SDL_FRect dst = { 
            std::floor(x + (i * 8 * scale)), 
            std::floor(y), 
            8.0f * scale, 
            8.0f * scale 
        };

        SDL_RenderTexture(renderer, fontTexture, &src, &dst);
    }
}

// --- Logic ---

void XenonGame::fireMissile() {
    if (m_missileCooldown > 0.0f) return;
    
    SDL_FRect shipRect = m_ship.getRect();
    float centerX = shipRect.x + shipRect.w * 0.5f - MISSILE_WIDTH * 0.5f;
    float topY    = shipRect.y - MISSILE_HEIGHT;

    auto spawn = [&](float offX, float velX) {
        Missile m;
        m.rect = {centerX + offX, topY, MISSILE_WIDTH, MISSILE_HEIGHT};
        m.src = {0, 0, 8, 16}; // Assume first frame of missile strip
        m.speedY = -MISSILE_SPEED;
        m.speedX = velX;
        m.alive = true;
        m_missiles.push_back(m);
    };

    spawn(0.0f, 0.0f);
    if (m_weaponLevel >= 1) { spawn(-15.0f, -50.0f); spawn(15.0f, 50.0f); }
    if (m_weaponLevel >= 2) { spawn(-30.0f, -150.0f); spawn(30.0f, 150.0f); }

    m_missileCooldown = 0.15f;
}

void XenonGame::updateMissiles(float dt) {
    for (auto& m : m_missiles) {
        if (!m.alive) continue;
        m.rect.y += m.speedY * dt;
        m.rect.x += m.speedX * dt;
        if (m.rect.y + m.rect.h < 0) m.alive = false;
    }
    m_missiles.erase(std::remove_if(m_missiles.begin(), m_missiles.end(), [](auto& m){ return !m.alive; }), m_missiles.end());
}

void XenonGame::renderMissiles(SDL_Renderer* r) {
    if(!m_missileTexture) return;
    for (const auto& m : m_missiles) SDL_RenderTexture(r, m_missileTexture, &m.src, &m.rect);
}

// Enemies
void XenonGame::spawnLoner() {
    if(!m_lonerTexture) return;
    Enemy e;
    e.type = EnemyType::Loner;
    e.src = {0.0f, 0.0f, 64.0f, 64.0f};
    e.rect = {0.0f, 80.0f, 64.0f, 64.0f};
    bool left = (randomFloat(0,1) > 0.5f);
    e.rect.x = left ? -70.0f : m_ctx.width + 10.0f;
    e.speedX = left ? 100.0f : -100.0f;
    e.speedY = 20.0f; e.hp = 2; e.alive = true; e.shootTimer = 1.0f;
    m_enemies.push_back(e);
}

void XenonGame::spawnRusher() {
    if(!m_rusherTexture) return;
    Enemy e;
    e.type = EnemyType::Rusher;
    e.src = {0.0f, 0.0f, 64.0f, 64.0f};
    e.rect = {randomFloat(50.0f, m_ctx.width - 100.0f), -70.0f, 64.0f, 64.0f};
    e.speedY = 300.0f; e.hp = 1; e.alive = true;
    m_enemies.push_back(e);
}

void XenonGame::updateEnemies(float dt) {
    for (auto& e : m_enemies) {
        if (!e.alive) continue;
        e.rect.x += e.speedX * dt;
        e.rect.y += e.speedY * dt;
        if (e.type == EnemyType::Loner) {
            e.shootTimer -= dt;
            if (e.shootTimer <= 0) {
                fireEnemyProjectile(e.rect, 250.0f);
                e.shootTimer = 1.5f;
            }
        }
        if (e.rect.y > m_ctx.height + 100 || e.rect.x < -100 || e.rect.x > m_ctx.width + 100) e.alive = false;
    }
    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(), [](auto& e){ return !e.alive; }), m_enemies.end());
}

void XenonGame::renderEnemies(SDL_Renderer* r) {
    for (const auto& e : m_enemies) {
        SDL_Texture* t = (e.type == EnemyType::Rusher) ? m_rusherTexture : m_lonerTexture;
        SDL_RenderTexture(r, t, &e.src, &e.rect);
    }
}

// Projectiles
void XenonGame::fireEnemyProjectile(const SDL_FRect& sourceRect, float speedY, float speedX) {
    if(!m_enemyProjectileTexture) return;
    EnemyProjectile p;
    p.rect = {sourceRect.x + sourceRect.w/2 - 4.0f, sourceRect.y + sourceRect.h, 8.0f, 8.0f};
    p.speedY = speedY; p.speedX = speedX; p.alive = true;
    m_enemyProjectiles.push_back(p);
}

void XenonGame::updateEnemyProjectiles(float dt) {
    for (auto& p : m_enemyProjectiles) {
        p.rect.x += p.speedX * dt;
        p.rect.y += p.speedY * dt;
        if (p.rect.y > m_ctx.height) p.alive = false;
    }
    m_enemyProjectiles.erase(std::remove_if(m_enemyProjectiles.begin(), m_enemyProjectiles.end(), [](auto& p){ return !p.alive; }), m_enemyProjectiles.end());
}

void XenonGame::renderEnemyProjectiles(SDL_Renderer* r) {
    if(!m_enemyProjectileTexture) return;
    for(const auto& p : m_enemyProjectiles) SDL_RenderTexture(r, m_enemyProjectileTexture, nullptr, &p.rect);
}

// Asteroids
void XenonGame::spawnAsteroid() {
    Asteroid a;
    int type = rand() % 3;
    float x = randomFloat(0.0f, m_ctx.width - 50.0f);
    a.animTimer = 0.0f; a.currentFrame = 0; a.totalFrames = 16; 

    if (type == 0 && m_asteroidSTexture) {
        a.size = AsteroidSize::Small;
        a.rect = {x, -64.0f, 32.0f, 32.0f};
        a.src = {0.0f, 0.0f, 32.0f, 32.0f};
        a.hp = 2;
    } else if (type == 1 && m_asteroidMTexture) {
        a.size = AsteroidSize::Medium;
        a.rect = {x, -64.0f, 64.0f, 64.0f};
        a.src = {0.0f, 0.0f, 64.0f, 64.0f};
        a.hp = 4;
    } else {
        a.size = AsteroidSize::Large;
        a.rect = {x, -96.0f, 96.0f, 96.0f};
        a.src = {0.0f, 0.0f, 96.0f, 96.0f};
        a.hp = 8;
    }
    a.speedY = randomFloat(80.0f, 150.0f); a.alive = true;
    m_asteroids.push_back(a);
}

void XenonGame::updateAsteroids(float dt) {
    for(auto& a : m_asteroids) {
        a.rect.y += a.speedY * dt;
        a.animTimer += dt;
        if (a.animTimer > 0.05f) {
            a.animTimer = 0.0f;
            a.currentFrame = (a.currentFrame + 1) % a.totalFrames;
            a.src.x = (float)a.currentFrame * a.src.w; // Shift source rect x
        }
        if(a.rect.y > m_ctx.height + 100) a.alive = false;
    }
    m_asteroids.erase(std::remove_if(m_asteroids.begin(), m_asteroids.end(), [](auto& a){ return !a.alive; }), m_asteroids.end());
}

void XenonGame::renderAsteroids(SDL_Renderer* r) {
    for(const auto& a : m_asteroids) {
        SDL_Texture* t = nullptr;
        if(a.size == AsteroidSize::Small) t = m_asteroidSTexture;
        else if(a.size == AsteroidSize::Medium) t = m_asteroidMTexture;
        else t = m_asteroidGTexture;
        if(t) SDL_RenderTexture(r, t, &a.src, &a.rect);
    }
}

// Boss
void XenonGame::spawnBoss() {
    m_boss.maxHp = 100; m_boss.hp = m_boss.maxHp;
    m_boss.rect = {m_ctx.width/2.0f - 64.0f, -150.0f, 128.0f, 128.0f};
    m_boss.active = true; m_boss.dirX = 100.0f; m_boss.shootTimer = 2.0f;
    m_enemies.clear(); m_asteroids.clear();
}

void XenonGame::updateBoss(float dt) {
    if(!m_boss.active) return;
    if(m_boss.rect.y < 50.0f) m_boss.rect.y += 50.0f * dt;
    else {
        m_boss.rect.x += m_boss.dirX * dt;
        if(m_boss.rect.x < 0 || m_boss.rect.x + m_boss.rect.w > m_ctx.width) m_boss.dirX *= -1.0f;
        m_boss.shootTimer -= dt;
        if(m_boss.shootTimer <= 0) {
            fireEnemyProjectile(m_boss.rect, 300.0f, 0.0f);
            fireEnemyProjectile(m_boss.rect, 250.0f, -100.0f);
            fireEnemyProjectile(m_boss.rect, 250.0f, 100.0f);
            m_boss.shootTimer = 1.5f;
        }
    }
}

void XenonGame::renderBoss(SDL_Renderer* r) {
    if(m_boss.active && m_bossTexture) {
        SDL_RenderTexture(r, m_bossTexture, nullptr, &m_boss.rect);
        // HP Bar
        SDL_FRect barBg = {m_boss.rect.x, m_boss.rect.y - 15.0f, m_boss.rect.w, 10.0f};
        SDL_SetRenderDrawColor(r, 50, 0, 0, 255); SDL_RenderFillRect(r, &barBg);
        float pct = (float)m_boss.hp / (float)m_boss.maxHp;
        SDL_FRect barFg = {m_boss.rect.x + 1.0f, m_boss.rect.y - 14.0f, (m_boss.rect.w - 2.0f) * pct, 8.0f};
        SDL_SetRenderDrawColor(r, 255, 50, 50, 255); SDL_RenderFillRect(r, &barFg);
    }
}

// PowerUps
void XenonGame::spawnPowerUp(float x, float y) {
    if (randomFloat(0,100) > 60) return;
    PowerUp p;
    p.rect = {x, y, 32.0f, 32.0f};
    p.src = {0.0f, 0.0f, 32.0f, 32.0f};
    p.alive = true; p.speedY = 100.0f;
    p.currentFrame = 0; p.totalFrames = 8; p.animTimer = 0.0f;

    int r = rand() % 10;
    if(r < 3) p.type = PowerUpType::Score;
    else if(r < 6) p.type = PowerUpType::Weapon;
    else if(r < 8) p.type = PowerUpType::Shield;
    else p.type = PowerUpType::Life;
    m_powerups.push_back(p);
}

void XenonGame::updatePowerUps(float dt) {
    for(auto& p : m_powerups) {
        p.rect.y += p.speedY * dt;
        p.animTimer += dt;
        if(p.animTimer > 0.1f) {
            p.animTimer = 0.0f;
            p.currentFrame = (p.currentFrame + 1) % p.totalFrames;
            p.src.x = (float)p.currentFrame * 32.0f;
        }
        if(p.rect.y > m_ctx.height) p.alive = false;
    }
    m_powerups.erase(std::remove_if(m_powerups.begin(), m_powerups.end(), [](auto& p){ return !p.alive; }), m_powerups.end());
}

void XenonGame::renderPowerUps(SDL_Renderer* r) {
    for(const auto& p : m_powerups) {
        SDL_Texture* t = nullptr;
        switch(p.type) {
            case PowerUpType::Weapon: t = m_puWeaponTexture; break;
            case PowerUpType::Shield: t = m_puShieldTexture; break;
            case PowerUpType::Life:   t = m_puLifeTexture; break;
            case PowerUpType::Score:  t = m_puScoreTexture; break;
        }
        if(t) SDL_RenderTexture(r, t, &p.src, &p.rect);
    }
}

void XenonGame::applyPowerUp(PowerUpType type) {
    switch(type) {
        case PowerUpType::Score: m_score += 500; break;
        case PowerUpType::Weapon: if(m_weaponLevel < 2) m_weaponLevel++; break;
        case PowerUpType::Life: m_lives++; break;
        case PowerUpType::Shield: m_hasShield = true; m_shieldTimer = SHIELD_DURATION; break;
    }
}

// Collisions
bool XenonGame::rectsOverlap(const SDL_FRect& a, const SDL_FRect& b) {
    return SDL_HasRectIntersectionFloat(&a, &b);
}

void XenonGame::checkCollisions() {
    SDL_FRect sRect = m_ship.getRect();
    sRect.x += 10; sRect.w -= 20; sRect.y += 10; sRect.h -= 20;

    // Missiles
    for(auto& m : m_missiles) {
        if(!m.alive) continue;
        for(auto& e : m_enemies) {
            if(!e.alive) continue;
            if(rectsOverlap(m.rect, e.rect)) {
                m.alive = false; e.hp--;
                if(e.hp <= 0) {
                    e.alive = false;
                    spawnExplosion(e.rect.x + 32, e.rect.y + 32);
                    spawnPowerUp(e.rect.x, e.rect.y);
                    m_score += 100;
                }
                break;
            }
        }
        if(!m.alive) continue;
        for(auto& a : m_asteroids) {
            if(!a.alive) continue;
            if(rectsOverlap(m.rect, a.rect)) {
                m.alive = false; a.hp--;
                if(a.hp <= 0) {
                    a.alive = false;
                    spawnExplosion(a.rect.x + a.rect.w/2, a.rect.y + a.rect.h/2);
                    m_score += 50;
                }
                break;
            }
        }
        if(m_boss.active && rectsOverlap(m.rect, m_boss.rect)) {
            m.alive = false; m_boss.hp--;
            if(m_boss.hp <= 0) {
                m_boss.active = false;
                spawnExplosion(m_boss.rect.x+64, m_boss.rect.y+64);
                m_gameState = GameState::Victory;
            }
        }
    }

    // Player Hits
    for(auto& p : m_enemyProjectiles) if(p.alive && rectsOverlap(p.rect, sRect)) { p.alive = false; onPlayerHit(); }
    for(auto& e : m_enemies) if(e.alive && rectsOverlap(e.rect, sRect)) { e.alive = false; onPlayerHit(); spawnExplosion(e.rect.x+32, e.rect.y+32); }
    for(auto& a : m_asteroids) if(a.alive && rectsOverlap(a.rect, sRect)) { a.alive = false; onPlayerHit(); spawnExplosion(a.rect.x+32, a.rect.y+32); }
    
    // Powerups
    for(auto& p : m_powerups) if(p.alive && rectsOverlap(p.rect, sRect)) { p.alive = false; applyPowerUp(p.type); }
}

void XenonGame::onPlayerHit() {
    if(m_hasShield) return;
    m_lives--;
    spawnExplosion(m_ship.getRect().x + 32, m_ship.getRect().y + 32);
    if(m_lives <= 0) m_gameState = GameState::GameOver;
    else { m_hasShield = true; m_shieldTimer = 2.0f; }
}

// Explosions
void XenonGame::spawnExplosion(float cx, float cy) {
    if(!m_explosionTexture) return;
    Explosion ex;
    ex.alive = true; ex.currentFrame = 0; ex.totalFrames = 8; ex.frameTime = 0;
    ex.dst = {cx - 32.0f, cy - 32.0f, 64.0f, 64.0f};
    ex.src = {0.0f, 0.0f, 64.0f, 64.0f};
    m_explosions.push_back(ex);
}

void XenonGame::updateExplosions(float dt) {
    for(auto& ex : m_explosions) {
        if(!ex.alive) continue;
        ex.frameTime += dt;
        if(ex.frameTime >= 0.05f) {
            ex.frameTime = 0;
            ex.currentFrame++;
            if(ex.currentFrame >= ex.totalFrames) ex.alive = false;
            ex.src.x = (float)ex.currentFrame * 64.0f;
        }
    }
    m_explosions.erase(std::remove_if(m_explosions.begin(), m_explosions.end(), [](auto& e){ return !e.alive; }), m_explosions.end());
}

void XenonGame::renderExplosions(SDL_Renderer* r) {
    for(const auto& ex : m_explosions) SDL_RenderTexture(r, m_explosionTexture, &ex.src, &ex.dst);
}

// Dust
void XenonGame::initDustBackground() {
    SDL_Texture* dusts[] = {
        m_ctx.textures->load("graphics/GDust.bmp"),
        m_ctx.textures->load("graphics/MDust.bmp"),
        m_ctx.textures->load("graphics/SDust.bmp")
    };
    for(int i=0; i<3; ++i) {
        if(!dusts[i]) continue;
        SDL_SetTextureAlphaMod(dusts[i], 150 + i*30);
        for(int j=0; j<20; ++j) {
            DustParticle p;
            p.texture = dusts[i];
            p.rect = {randomFloat(0, m_ctx.width), randomFloat(0, m_ctx.height), 32.0f, 32.0f};
            p.src = {0, 0, 32, 32}; 
            p.speed = 50.0f + i*20.0f;
            m_dustParticles.push_back(p);
        }
    }
}

void XenonGame::updateDust(float dt) {
    for(auto& p : m_dustParticles) {
        p.rect.y += p.speed * dt;
        if(p.rect.y > m_ctx.height) p.rect.y = -32.0f;
    }
}

void XenonGame::renderDust(SDL_Renderer* r) {
    for(const auto& p : m_dustParticles) SDL_RenderTexture(r, p.texture, nullptr, &p.rect);
}

// HUD
void XenonGame::drawText(SDL_Renderer* r, float x, float y, const std::string& text) {
    if(!m_fontTexture) return;
    SDL_FRect dst = {x, y, 16.0f, 16.0f};
    SDL_FRect src = {0.0f, 0.0f, 8.0f, 8.0f};
    for(char c : text) {
        int idx = (unsigned char)c - 32; if(idx < 0) idx = 0;
        src.x = (idx % 16) * 8.0f;
        src.y = (idx / 16) * 8.0f;
        SDL_RenderTexture(r, m_fontTexture, &src, &dst);
        dst.x += 16.0f;
    }
}

void XenonGame::renderHUD(SDL_Renderer* r) {
    drawText(r, 10, 10, "SCORE:" + std::to_string(m_score));
    
    // Shield Bar (Green)
    float barW = 200.0f;
    SDL_FRect bg = {m_ctx.width - barW - 20, 10, barW, 20};
    SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
    SDL_RenderFillRect(r, &bg);
    if(m_hasShield) {
        float pct = m_shieldTimer / SHIELD_DURATION;
        SDL_FRect fg = {bg.x+2, bg.y+2, (barW-4)*pct, 16};
        SDL_SetRenderDrawColor(r, 0, 255, 0, 255);
        SDL_RenderFillRect(r, &fg);
        drawText(r, bg.x, bg.y + 25, "SHIELD");
    } else {
        drawText(r, bg.x, bg.y + 25, "HULL");
    }

    if(m_gameState == GameState::GameOver) drawText(r, m_ctx.width/2-80, m_ctx.height/2, "GAME OVER - PRESS R");
    if(m_gameState == GameState::Victory) drawText(r, m_ctx.width/2-80, m_ctx.height/2, "VICTORY! - PRESS R");
}

void XenonGame::renderText(SDL_Renderer* renderer, const std::string& text, float x, float y)
{
    if (!m_fontTexture) return;

    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        // Predpoklad: Font začína od medzery (ASCII 32) a má mriežku 8x8
        int index = static_cast<int>(c) - 32;
        if (index < 0) index = 0;

        SDL_FRect src;
        src.x = static_cast<float>(index * 8);
        src.y = 0.0f;
        src.w = 8.0f;
        src.h = 8.0f;

        SDL_FRect dst;
        dst.x = std::floor(x + (i * 8)); // floor je dôležitý proti rozmazaniu
        dst.y = std::floor(y);
        dst.w = 8.0f;
        dst.h = 8.0f;

        SDL_RenderTexture(renderer, m_fontTexture, &src, &dst);
    }
}