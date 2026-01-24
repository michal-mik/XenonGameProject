#include "Engine/Engine.hpp"
#include "XenonGame.hpp"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    XenonGame game;
    Engine engine(800, 600, "AGPT Project 1 - Xenon 2000", game);

    if (!engine.init()) {
        return 1;
    }

    engine.run();
    return 0;
}