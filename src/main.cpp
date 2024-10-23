#include <iostream>
#include <cstring>

#include "raylib.h"
#include "raymath.h"

#include <oscpp/print.hpp>
#include "OscServer.hpp"

float FPS = 60.f;
float FRAME_TIME = 1. / FPS;
Vector2 wAspect {16, 9};
Vector2 wSize {1280, 1280 / wAspect.x * wAspect.y};

int main()
{
    OscServer server(3333);

    InitWindow(wSize.x, wSize.y, "Osc Cpp");
    SetTargetFPS(FPS);

    Vector2 mouse {0,0};
    int mouseZ = 0;
    int frameCounter = 0;

    while (!WindowShouldClose()) {
        double now = GetTime();
        mouse = GetMousePosition();
        mouseZ = IsMouseButtonPressed(0) ? 1 : (IsMouseButtonPressed(1) ? -1 : 0);

        BeginDrawing();
            ClearBackground(SKYBLUE);

            try {
                server.recv([](OSCPP::Server::Message msg) {
                    OSCPP::Server::ArgStream args = msg.args();
                    std::cout << "Received: " << msg << std::endl;
                });
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }



            DrawText(TextFormat("%ims / %ifps", (int)((GetTime() - now) * 1000.), GetFPS()), 20, 20, 30, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}