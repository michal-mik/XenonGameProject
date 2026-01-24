#include "Engine/Engine.hpp"


#include <iostream>

Engine::Engine(int width, int height, const std::string& title, IGame& game)
    : m_width(width)
    , m_height(height)
    , m_title(title)
    , m_game(game)
{
}

Engine::~Engine()
{
    shutdown();
}

bool Engine::init()
{
    if (!initSDL()) {
        std::cerr << "[Engine] Failed to init SDL\n";
        return false;
    }

    if (!initBox2D()) {
        std::cerr << "[Engine] Failed to init Box2D\n";
        return false;
    }

    m_textureManager.setRenderer(m_renderer);

    m_ctx.window   = m_window;
    m_ctx.renderer = m_renderer;
    m_ctx.world    = m_world;
    m_ctx.width    = m_width;
    m_ctx.height   = m_height;
    m_ctx.textures = &m_textureManager;


    if (!m_game.init(m_ctx)) {
        std::cerr << "[Engine] Game init failed\n";
        return false;
    }

    return true;
}

bool Engine::initSDL()
{
    // include gamepads for that 5% mark later
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        std::cerr << "[Engine] SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    m_window = SDL_CreateWindow(
        m_title.c_str(),
        m_width,
        m_height,
        0 // window flags
    );

    if (!m_window) {
        std::cerr << "[Engine] SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    // SDL3: two parameters, then enable vsync separately
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "[Engine] SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!SDL_SetRenderVSync(m_renderer, 1)) {
        std::cerr << "[Engine] SDL_SetRenderVSync failed: "
                  << SDL_GetError() << "\n";
        // not fatal
    }

    return true;
}

bool Engine::initBox2D()
{
    b2WorldDef worldDef = b2DefaultWorldDef();

    // Top-down game → no gravity
    worldDef.gravity = (b2Vec2){0.0f, 0.0f};

    m_world = b2CreateWorld(&worldDef);
    if (B2_IS_NULL(m_world)) {
        std::cerr << "[Engine] Failed to create Box2D world\n";
        return false;
    }

    std::cout << "[Engine] Box2D world initialized\n";
    return true;
}

void Engine::run()
{
    bool running = true;
    uint64_t lastTicks = SDL_GetTicks();

    while (running) {
        uint64_t currentTicks = SDL_GetTicks();
        float dt = static_cast<float>(currentTicks - lastTicks) / 1000.0f;
        lastTicks = currentTicks;

        processEvents(running);
        update(dt);
        render();
    }
}

void Engine::processEvents(bool& running)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            running = false;
        }

        // Forward everything to the game
        m_game.handleEvent(e, running);
    }
}

void Engine::update(float dt)
{
    // fixed-step physics
    m_accumulator += dt;
    const float timeStep = s_fixedTimeStep;
    const int   subSteps = 4;

    while (m_accumulator >= timeStep) {
        if (!B2_IS_NULL(m_world)) {
            b2World_Step(m_world, timeStep, subSteps);
        }
        m_accumulator -= timeStep;
    }

    // game logic
    m_game.update(dt);
}

void Engine::render()
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    m_game.render(m_renderer);

    SDL_RenderPresent(m_renderer);
}

void Engine::shutdown()
{
    m_textureManager.clear();

    if (B2_IS_NON_NULL(m_world)) {
        b2DestroyWorld(m_world);
        m_world = b2_nullWorldId;
    }

    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}
