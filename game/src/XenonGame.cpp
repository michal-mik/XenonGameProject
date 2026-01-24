#include "XenonGame.hpp"
#include "ShipPawn.hpp"
#include "Engine/TextureManager.hpp" 

#include <iostream>
#include <random>

namespace {
    constexpr float MISSILE_WIDTH  = 8.0f;    // tweak to match missile.bmp
    constexpr float MISSILE_HEIGHT = 16.0f;   // tweak if needed
    constexpr float MISSILE_SPEED  = 500.0f;  // pixels per second
}

XenonGame::~XenonGame()
{
    if (m_gamepad) {
        SDL_CloseGamepad(m_gamepad);
        m_gamepad = nullptr;
    }
}

bool XenonGame::init(const EngineContext& ctx)
{
    m_ctx = ctx;

    if (!m_ctx.textures) {
        std::cerr << "[Game] EngineContext.textures is null\n";
        return false;
    }

    // Init the ship pawn (uses TextureManager internally)
    if (!m_ship.init(m_ctx.textures,
                     "Ship1.bmp",
                     SHIP_FRAME_WIDTH,
                     SHIP_FRAME_HEIGHT,
                     m_ctx.width,
                     m_ctx.height)) {
        std::cout << "[Game] Failed to init ship pawn\n";
        return false;
    }

    // Load game textures
    m_missileTexture         = m_ctx.textures->load("graphics/missile.bmp");
    m_lonerTexture           = m_ctx.textures->load("graphics/LonerA.bmp");
    m_rusherTexture          = m_ctx.textures->load("graphics/rusher.bmp");
    m_enemyProjectileTexture = m_ctx.textures->load("graphics/EnWeap6.bmp");
    m_explosionTexture       = m_ctx.textures->load("graphics/explode64.bmp");

    m_ship.setSpeed(300.0f);

    if (!m_missileTexture || !m_lonerTexture ||
        !m_rusherTexture || !m_enemyProjectileTexture ||
        !m_explosionTexture) {
        std::cout << "[Game] Failed to load one or more game textures\n";
        return false;
    }

    // initialise enemy spawn timers
    m_lonerSpawnTimer  = LONER_SPAWN_INTERVAL * 0.5f; // first loner appears sooner
    m_rusherSpawnTimer = RUSHER_SPAWN_INTERVAL;

    // parallax background (dust)
    initDustBackground();

    return true;
}

// ------------------------------------------------------------
// input
// ------------------------------------------------------------
void XenonGame::handleEvent(const SDL_Event& e, bool& running)
{
    switch (e.type) {
    case SDL_EVENT_KEY_DOWN:
    if (e.key.repeat) break;

    switch (e.key.key) {
    case SDLK_ESCAPE: running = false;      break;
    case SDLK_LEFT:
    case SDLK_A:     m_ship.setMoveLeft(true);  break;
    case SDLK_RIGHT:
    case SDLK_D:     m_ship.setMoveRight(true); break;
    case SDLK_UP:
    case SDLK_W:     m_ship.setMoveUp(true);    break;
    case SDLK_DOWN:
    case SDLK_S:     m_ship.setMoveDown(true);  break;
    case SDLK_SPACE: fireMissile();             break;
    default: break;
    }
    break;

case SDL_EVENT_KEY_UP:
    switch (e.key.key) {
    case SDLK_LEFT:
    case SDLK_A:     m_ship.setMoveLeft(false);  break;
    case SDLK_RIGHT:
    case SDLK_D:     m_ship.setMoveRight(false); break;
    case SDLK_UP:
    case SDLK_W:     m_ship.setMoveUp(false);    break;
    case SDLK_DOWN:
    case SDLK_S:     m_ship.setMoveDown(false);  break;
    default: break;
    }
    break;


    // --- gamepad plugged in -------------------------------------------
    case SDL_EVENT_GAMEPAD_ADDED: {
        if (!m_gamepad) {
            SDL_JoystickID jid = e.gdevice.which;

            SDL_Gamepad* gp = SDL_OpenGamepad(jid);
            if (gp) {
                m_gamepad = gp;
                m_gamepadConnected = true;
                std::cout << "[Game] Gamepad connected\n";
            } else {
                std::cerr << "[Game] SDL_OpenGamepad failed: "
                          << SDL_GetError() << "\n";
            }
        }
        break;
    }

    // --- gamepad unplugged --------------------------------------------
    case SDL_EVENT_GAMEPAD_REMOVED: {
        if (m_gamepad &&
            SDL_GetGamepadID(m_gamepad) == e.gdevice.which) {
            SDL_CloseGamepad(m_gamepad);
            m_gamepad = nullptr;
            m_gamepadConnected = false;
            std::cout << "[Game] Gamepad disconnected\n";
        }
        break;
    }

    // --- face buttons (A/Cross etc.) ----------------------------------
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        if (!m_gamepad) break;

        const bool down = e.gbutton.down;
        const SDL_GamepadButton btn =
            static_cast<SDL_GamepadButton>(e.gbutton.button);

        // South button = A on Xbox, Cross on PlayStation
        if (btn == SDL_GAMEPAD_BUTTON_SOUTH && down) {
            // same as pressing space: fire a missile
            fireMissile();
        }
        break;
    }

    // --- left stick for movement --------------------------------------
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
    if (!m_gamepad) break;

    const SDL_GamepadAxis axis =
        static_cast<SDL_GamepadAxis>(e.gaxis.axis);
    const float value =
        static_cast<float>(e.gaxis.value) / 32767.0f;
    const float dz = m_gamepadDeadzone;

        if (axis == SDL_GAMEPAD_AXIS_LEFTX) {
            if (value > dz) {
                m_ship.setMoveRight(true);
                m_ship.setMoveLeft(false);
            } else if (value < -dz) {
                m_ship.setMoveLeft(true);
                m_ship.setMoveRight(false);
            } else {
                m_ship.setMoveLeft(false);
                m_ship.setMoveRight(false);
            }
        } else if (axis == SDL_GAMEPAD_AXIS_LEFTY) {
            if (value > dz) {
                m_ship.setMoveDown(true);
                m_ship.setMoveUp(false);
            } else if (value < -dz) {
                m_ship.setMoveUp(true);
                m_ship.setMoveDown(false);
            } else {
                m_ship.setMoveUp(false);
                m_ship.setMoveDown(false);
            }
        }
        break;
    }

    default:
        break;
    }  
}

// ------------------------------------------------------------
// update
// ------------------------------------------------------------

void XenonGame::update(float dt)
{
    // --- background -------------------------------------------------------
    updateDust(dt);

    // --- ship movement ----------------------------------------------------
    m_ship.update(dt);

    // clamp ship to window
    SDL_FRect shipRect = m_ship.getRect();

    if (shipRect.x < 0.0f) shipRect.x = 0.0f;
    if (shipRect.x + shipRect.w > m_ctx.width)
        shipRect.x = static_cast<float>(m_ctx.width) - shipRect.w;

    if (shipRect.y < 0.0f) shipRect.y = 0.0f;
    if (shipRect.y + shipRect.h > m_ctx.height)
        shipRect.y = static_cast<float>(m_ctx.height) - shipRect.h;

    m_ship.setPosition(shipRect.x, shipRect.y);

    // --- missile cooldown -------------------------------------------------
    if (m_missileCooldown > 0.0f) {
        m_missileCooldown -= dt;
        if (m_missileCooldown < 0.0f) m_missileCooldown = 0.0f;
    }

    // --- spawn timers -----------------------------------------------------
    m_lonerSpawnTimer -= dt;
    if (m_lonerSpawnTimer <= 0.0f) {
        spawnLoner();
        m_lonerSpawnTimer = m_levelConfig.lonerInterval;
    }

    m_rusherSpawnTimer -= dt;
    if (m_rusherSpawnTimer <= 0.0f) {
        spawnRusher();
        m_rusherSpawnTimer = m_levelConfig.rusherInterval;
    }

    // --- main game logic --------------------------------------------------
    updateMissiles(dt);
    updateEnemies(dt);
    updateEnemyProjectiles(dt);
    handleMissileEnemyCollisions();
    handleEnemyProjectileShipCollisions();
    updateExplosions(dt);
}



// ------------------------------------------------------------
// render
// ------------------------------------------------------------
void XenonGame::render(SDL_Renderer* renderer)
{
    // parallax background first
    renderDust(renderer);

    // then ship
    m_ship.render(renderer);

    // enemies + bullets
    renderEnemies(renderer);
    renderEnemyProjectiles(renderer);

    // player missiles + explosions on top
    renderMissiles(renderer);
    renderExplosions(renderer);
}



// ------------------------------------------------------------
// missiles
// ------------------------------------------------------------
void XenonGame::fireMissile()
{
    if (m_missileCooldown > 0.0f) {
        return;
    }

    if (!m_missileTexture) {
        return;
    }

    Missile m;
    m.rect.w = MISSILE_WIDTH;
    m.rect.h = MISSILE_HEIGHT;

    // get ship rect from pawn
    SDL_FRect shipRect = m_ship.getRect();

    // spawn missile at top-center of the ship
    m.rect.x = shipRect.x + shipRect.w * 0.5f - m.rect.w * 0.5f;
    m.rect.y = shipRect.y - m.rect.h;

    m.speedY = -MISSILE_SPEED;
    m.alive  = true;

    m_missiles.push_back(m);

    m_missileCooldown = 0.1f; // adjust to taste
}

void XenonGame::updateMissiles(float dt)
{
    for (auto& m : m_missiles) {
        if (!m.alive) continue;

        m.rect.y += m.speedY * dt;

        // if missile goes off top of the screen, kill it
        if (m.rect.y + m.rect.h < 0.0f) {
            m.alive = false;
        }
    }

    // optional: compact vector to remove dead ones
    m_missiles.erase(
        std::remove_if(m_missiles.begin(), m_missiles.end(),
                       [](const Missile& m) { return !m.alive; }),
        m_missiles.end()
    );
}


void XenonGame::renderMissiles(SDL_Renderer* renderer)
{
    if (!m_missileTexture) return;

    for (const auto& m : m_missiles) {
        if (!m.alive) continue;

        // nullptr = use whole texture as source
        SDL_RenderTexture(renderer, m_missileTexture, nullptr, &m.rect);
    }
}

// ------------------------------------------------------------
// enemies (Loners + Rushers)
// ------------------------------------------------------------
void XenonGame::spawnLoner()
{
    if (!m_lonerTexture) return;

    Enemy e{};
    e.type  = EnemyType::Loner;
    e.alive = true;

    e.src.x = 0.0f;
    e.src.y = 0.0f;
    e.src.w = static_cast<float>(LONER_FRAME_WIDTH);
    e.src.h = static_cast<float>(LONER_FRAME_HEIGHT);

    e.rect.w = e.src.w;
    e.rect.h = e.src.h;
    e.rect.y = 80.0f;

    static bool fromLeft = true;

    if (fromLeft) {
        e.rect.x = -e.rect.w;
        e.speedX = 80.0f;
    } else {
        e.rect.x = static_cast<float>(m_ctx.width);
        e.speedX = -80.0f;
    }

    e.speedY    = 10.0f;
    e.shootTimer = 0.8f;   // first shot after spawn

    fromLeft = !fromLeft;

    m_enemies.push_back(e);
}

void XenonGame::spawnRusher()
{
    if (!m_rusherTexture) return;

    Enemy e{};
    e.type  = EnemyType::Rusher;
    e.alive = true;

    e.src.x = 0.0f;
    e.src.y = 0.0f;
    e.src.w = static_cast<float>(RUSHER_FRAME_WIDTH);
    e.src.h = static_cast<float>(RUSHER_FRAME_HEIGHT);

    e.rect.w = e.src.w;
    e.rect.h = e.src.h;

    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> distX(
        0.0f, static_cast<float>(m_ctx.width) - e.rect.w
    );

    e.rect.x = distX(rng);
    e.rect.y = -e.rect.h;

    e.speedX     = 0.0f;
    e.speedY     = 120.0f;
    e.shootTimer = 0.0f; // Rushers don’t shoot in this version

    m_enemies.push_back(e);
}

void XenonGame::updateEnemies(float dt)
{
    for (auto& e : m_enemies) {
        if (!e.alive) continue;

        e.rect.x += e.speedX * dt;
        e.rect.y += e.speedY * dt;

        if (e.type == EnemyType::Loner) {
            e.shootTimer -= dt;
            if (e.shootTimer <= 0.0f) {
                fireEnemyProjectile(e);
                e.shootTimer = std::max(0.6f, m_levelConfig.lonerInterval);
            }
        }

        if (e.rect.x > m_ctx.width || e.rect.x + e.rect.w < 0.0f ||
            e.rect.y > m_ctx.height) {
            e.alive = false;
        }
    }

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
                       [](const Enemy& e) { return !e.alive; }),
        m_enemies.end()
    );
}

void XenonGame::renderEnemies(SDL_Renderer* renderer)
{
    for (const auto& e : m_enemies) {
        if (!e.alive) continue;

        SDL_Texture* tex = nullptr;
        switch (e.type) {
        case EnemyType::Loner:  tex = m_lonerTexture;  break;
        case EnemyType::Rusher: tex = m_rusherTexture; break;
        }

        if (!tex) continue;

        SDL_RenderTexture(renderer, tex, &e.src, &e.rect);
    }
}

// ------------------------------------------------------------
// enemy projectiles (fired by Loners)
// ------------------------------------------------------------
void XenonGame::fireEnemyProjectile(const Enemy& from)
{
    if (!m_enemyProjectileTexture) return;

    EnemyProjectile p{};

    float w = 0.0f, h = 0.0f;
    if (!SDL_GetTextureSize(m_enemyProjectileTexture, &w, &h)) {
        w = 8.0f;
        h = 8.0f;
    }

    p.rect.w = w;
    p.rect.h = h;

    // spawn from bottom-center of the enemy
    p.rect.x = from.rect.x + from.rect.w * 0.5f - p.rect.w * 0.5f;
    p.rect.y = from.rect.y + from.rect.h;

    p.speedY = ENEMY_PROJECTILE_SPEED;
    p.alive  = true;

    m_enemyProjectiles.push_back(p);
}

void XenonGame::updateEnemyProjectiles(float dt)
{
    for (auto& p : m_enemyProjectiles) {
        if (!p.alive) continue;

        p.rect.y += p.speedY * dt;

        if (p.rect.y > m_ctx.height) {
            p.alive = false;
        }
    }

    m_enemyProjectiles.erase(
        std::remove_if(m_enemyProjectiles.begin(), m_enemyProjectiles.end(),
                       [](const EnemyProjectile& p) { return !p.alive; }),
        m_enemyProjectiles.end()
    );
}

void XenonGame::renderEnemyProjectiles(SDL_Renderer* renderer)
{
    if (!m_enemyProjectileTexture) return;

    for (const auto& p : m_enemyProjectiles) {
        if (!p.alive) continue;
        SDL_RenderTexture(renderer, m_enemyProjectileTexture, nullptr, &p.rect);
    }
}


// ------------------------------------------------------------
// explosions
// ------------------------------------------------------------
void XenonGame::spawnExplosion(float cx, float cy)
{
    if (!m_explosionTexture) return;

    Explosion ex{};
    ex.alive        = true;
    ex.currentFrame = 0;
    ex.totalFrames  = EXPLOSION_TOTAL_FRAMES;
    ex.frameTime    = 0.0f;

    ex.dst.w = static_cast<float>(EXPLOSION_FRAME_SIZE);
    ex.dst.h = static_cast<float>(EXPLOSION_FRAME_SIZE);
    ex.dst.x = cx - ex.dst.w * 0.5f;
    ex.dst.y = cy - ex.dst.h * 0.5f;

    ex.src.w = ex.dst.w;
    ex.src.h = ex.dst.h;
    ex.src.x = 0.0f;
    ex.src.y = 0.0f;

    m_explosions.push_back(ex);
}

void XenonGame::updateExplosions(float dt)
{
    const float frameDuration = 1.0f / EXPLOSION_FPS;

    for (auto& ex : m_explosions) {
        if (!ex.alive) continue;

        ex.frameTime += dt;
        while (ex.frameTime >= frameDuration) {
            ex.frameTime -= frameDuration;
            ex.currentFrame++;

            if (ex.currentFrame >= ex.totalFrames) {
                ex.alive = false;
                break;
            }

            ex.src.x = static_cast<float>(
                ex.currentFrame * EXPLOSION_FRAME_SIZE
            );
        }
    }

    m_explosions.erase(
        std::remove_if(m_explosions.begin(), m_explosions.end(),
                       [](const Explosion& e) { return !e.alive; }),
        m_explosions.end()
    );
}

void XenonGame::renderExplosions(SDL_Renderer* renderer)
{
    if (!m_explosionTexture) return;

    for (const auto& ex : m_explosions) {
        if (!ex.alive) continue;
        SDL_RenderTexture(renderer, m_explosionTexture, &ex.src, &ex.dst);
    }
}

// ------------------------------------------------------------
// level background: parallax dust
// ------------------------------------------------------------
void XenonGame::initDustBackground()
{
    std::cout << "[Game] initDustBackground()\n";

    if (!m_ctx.textures) {
        std::cerr << "[Game] Cannot init dust: no TextureManager\n";
        return;
    }

    if (!m_dustParticles.empty()) {
        return;
    }

    SDL_Texture* gTex = m_ctx.textures->load("graphics/GDust.bmp");
    SDL_Texture* mTex = m_ctx.textures->load("graphics/MDust.bmp");
    SDL_Texture* sTex = m_ctx.textures->load("graphics/SDust.bmp");

    if (!gTex || !mTex || !sTex) {
        std::cerr << "[Game] Failed to load dust textures\n";
        return;
    }

    // make dust a bit transparent
    SDL_SetTextureAlphaMod(gTex, 140);
    SDL_SetTextureAlphaMod(mTex, 170);
    SDL_SetTextureAlphaMod(sTex, 200);

    // get texture sizes
    auto texSize = [](SDL_Texture* tex) -> SDL_FPoint {
        float w = 0.0f, h = 0.0f;
        if (!SDL_GetTextureSize(tex, &w, &h)) {
            w = 32.0f;
            h = 8.0f;
        }
        return SDL_FPoint{w, h};
    };

    SDL_FPoint gSize = texSize(gTex);
    SDL_FPoint mSize = texSize(mTex);
    SDL_FPoint sSize = texSize(sTex);

    static std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> distY(
        0.0f, static_cast<float>(m_ctx.height)
    );

    auto spawnLayer = [&](SDL_Texture* tex,
                          const SDL_FPoint& size,
                          int count,
                          float speed)
    {
        float tileW = size.x * 0.7f;
        float tileH = size.y;

        std::uniform_real_distribution<float> distX(
            0.0f, static_cast<float>(m_ctx.width) - tileW
        );

        for (int i = 0; i < count; ++i) {
            DustParticle p;
            p.texture = tex;
            p.rect.w  = tileW;
            p.rect.h  = tileH;
            p.rect.x  = distX(rng);
            p.rect.y  = distY(rng);
            p.speed   = speed;
            m_dustParticles.push_back(p);
        }
    };

    // fewer, softer particles
    spawnLayer(gTex, gSize, 12, 20.0f);   // far layer
    spawnLayer(mTex, mSize, 18, 40.0f);   // mid layer
    spawnLayer(sTex, sSize, 24, 70.0f);   // near layer

    std::cout << "[Game] dust particles: " << m_dustParticles.size() << "\n";
}

void XenonGame::updateDust(float dt)
{
    for (auto& p : m_dustParticles) {
        p.rect.y += p.speed * dt;

        if (p.rect.y > m_ctx.height) {
            p.rect.y -= static_cast<float>(m_ctx.height) + p.rect.h;
        }
    }
}

void XenonGame::renderDust(SDL_Renderer* renderer)
{
    for (const auto& p : m_dustParticles) {
        if (!p.texture) continue;
        SDL_RenderTexture(renderer, p.texture, nullptr, &p.rect);
    }
}

// ------------------------------------------------------------
// collisions
// ------------------------------------------------------------
bool XenonGame::rectsOverlap(const SDL_FRect& a, const SDL_FRect& b)
{
    return !(a.x > b.x + b.w ||
             a.x + a.w < b.x ||
             a.y > b.y + b.h ||
             a.y + a.h < b.y);
}

void XenonGame::handleMissileEnemyCollisions()
{
    for (auto& e : m_enemies) {
        if (!e.alive) continue;

        for (auto& m : m_missiles) {
            if (!m.alive) continue;

            if (rectsOverlap(e.rect, m.rect)) {
                e.alive = false;
                m.alive = false;

                float cx = e.rect.x + e.rect.w * 0.5f;
                float cy = e.rect.y + e.rect.h * 0.5f;
                spawnExplosion(cx, cy);
                break;
            }

        }
    }
}

void XenonGame::handleEnemyProjectileShipCollisions()
{
    SDL_FRect shipRect = m_ship.getRect();

    for (auto& p : m_enemyProjectiles) {
        if (!p.alive) continue;

        if (rectsOverlap(shipRect, p.rect)) {
            p.alive = false;

            float cx = shipRect.x + shipRect.w * 0.5f;
            float cy = shipRect.y + shipRect.h * 0.5f;
            spawnExplosion(cx, cy);

            std::cout << "[Game] Ship hit by enemy projectile!\n";
        }
    }
}

