#pragma once

#include <string>

#include <SDL3/SDL.h>
#include <box2d/box2d.h>

#include "Engine/TextureManager.hpp"


// Info the engine gives to the game during init
struct EngineContext {
    SDL_Window*    window   = nullptr;
    SDL_Renderer*  renderer = nullptr;
    b2WorldId      world    = b2_nullWorldId;
    int            width    = 0;
    int            height   = 0;
    TextureManager* textures = nullptr;
};


// Interface that the game must implement
class IGame {
public:
    virtual ~IGame() = default;

    // Called once after the engine initializes SDL + Box2D
    virtual bool init(const EngineContext& ctx) = 0;

    // Pass every SDL event to the game.
    // 'running' lets the game ask the engine to quit.
    virtual void handleEvent(const SDL_Event& e, bool& running) = 0;

    // Called every frame with delta time
    virtual void update(float dt) = 0;

    // Called every frame to draw
    virtual void render(SDL_Renderer* renderer) = 0;
};

// ------------------------------------------------------------
// Engine: lives in the engine library
// ------------------------------------------------------------
class Engine {
public:
    Engine(int width, int height, const std::string& title, IGame& game);
    ~Engine();

    bool init();
    void run();
    void shutdown();

private:
    bool initSDL();
    bool initBox2D();

    void processEvents(bool& running);
    void update(float dt);
    void render();

    int         m_width;
    int         m_height;
    std::string m_title;

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    b2WorldId m_world = b2_nullWorldId;

    float                    m_accumulator    = 0.0f;
    static constexpr float   s_fixedTimeStep  = 1.0f / 60.0f;

    EngineContext  m_ctx{};
    IGame&         m_game;
    TextureManager m_textureManager;
};