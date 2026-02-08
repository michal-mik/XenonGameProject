**AGPT Project Report -- Xenon 2000 clone**

Course: Advanced Game Programming Topics

Assignment: Game Project -- SDL3 + Box2D

Author: Michal Mikulka

Student ID: 2025225055

Date: 28.01.2026

Tools & Technologies: SDL3, Box2D, CMake, Xcode, C++

1.  **Introduction**

The goal of this assignment was to develop a clone of the classic arcade shooter *Xenon 2000* using C++ and the SDL3 library. The project focuses on creating a functional game engine architecture that separates core systems (rendering, physics initialization, input) from gameplay logic. While Box2D was integrated into the engine core for physics world management, specific gameplay mechanics such as projectile movement and collision detection were implemented using lightweight AABB (Axis-Aligned Bounding Box) logic to maintain performance and control for this arcade-style gameplay.

2.  **Objectives**

- E**ngine Architecture:** Create a reusable Engine class that handles SDL3 initialization, window creation, and the main application loop.

- **Gameplay Mechanics:** Implement a top-down shooter with a player ship, multiple enemy types (Loners and Rushers), and weapon systems.

- **Input Handling:** Support both Keyboard (WASD/Arrow keys) and Gamepad (SDL_Gamepad) for movement and firing.

- **Visuals:** Implement sprite rendering with animation states (banking left/right) and a multi-layered parallax scrolling background.

3.  **Game Engine design**

The architecture is divided into two primary layers: the generic Engine and the specific Game logic.

1.  The Engine Layer (Engine.cpp):

- Responsible for initializing SDL3 (Video and Gamepad subsystems) and creating the Box2D world.

- Manages the high-level application loop, including calculating delta time for frame-independent movement and stepping the physics world.

- Forwards input events and render calls to the generic IGame interface.

2.  The Game Layer (XenonGame.cpp):

- Implements the gameplay specific to Xenon 2000.

- Manages game entities (Player, Enemies, Projectiles, Particles) using std::vector containers.

- Handles collision detection using SDL_FRect overlap checks.

- Manages game state, such as enemy spawn timers and missile cooldowns.

4.  **Classes**

Engine - The core system class. It owns the SDL_Window and SDL_Renderer. It enforces a fixed-time-step update loop for physics consistency and manages the TextureManager lifecycle.

XenonGame - The concrete implementation of the IGame interface. This class acts as the \"World\" for the game entities.

- Spawning: Handles logic for spawning \"Loner\" enemies (horizontal movement, shooting) and \"Rusher\" enemies (vertical suicide dives).

- Background: Implements a 3-layer parallax dust system (DustParticle) to create a sense of depth and speed.

- Collision: Contains logic to detect overlaps between missiles, enemies, and the player ship, spawning Explosion entities upon destruction.

ShipPawn - Represents the player\'s entity.

- Visuals: Uses a sprite sheet to render different banking animations based on horizontal velocity (turning left, right, or staying idle).

- Movement: Processes input flags set by the Game class to calculate velocity.

- Init: Loads the specific ship texture via the TextureManager.

Structures (Internal) - Instead of heavy classes for every projectile, lightweight structs were used for transient entities:

- Missile / EnemyProjectile: Contains simple position (SDL_FRect), speed, and an alive flag.

- Enemy: Stores type (Loner/Rusher), movement timers, and texture references.

- Explosion: Handles frame-based animation for explosion effects using the explode64.bmp sprite sheet.

5.  **Key Implementation Details**

Input System - The project supports robust input handling. XenonGame::handleEvent listens for standard SDL keyboard events but also implements SDL_EVENT_GAMEPAD\_\* events. This allows for analog stick control with deadzone handling and face-button firing, creating a console-like experience.

Parallax Background - To achieve the classic scrolling space effect, initDustBackground() generates three layers of \"dust\" particles. Each layer moves at a different speed (20px, 40px, and 70px per second) and has different transparency values (alpha mod), simulating depth.

Collision Detection - While Box2D is initialized in the engine, the game uses optimized AABB collision (rectsOverlap) for the high volume of bullets and enemies. This checks if the bounding rectangles of projectiles and ships intersect, triggering destruction and explosion effects immediately.

6.  **Conclusion**

This project successfully demonstrates the integration of SDL3 into a custom C++ game engine. The result is a playable vertical shooter that replicates the feel of Xenon 2000 through responsive controls, parallax scrolling, and reactive enemy behaviors. Future improvements could include utilizing the initialized Box2D world for more complex physics interactions (like bouncing off walls) and adding audio support for weapon fire and explosions.
