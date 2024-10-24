#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>

#include "raylib.h"
// #include "raymath.h"


#include <oscpp/print.hpp>
#include "OscServer.hpp"

#include "log.hpp"
#include "ogl.hpp"

float FPS = 60.f;
float FRAME_TIME = 1. / FPS;
Vector2 wAspect {16, 9};
Vector2 wSize {1280, 1280 / wAspect.x * wAspect.y};

void DrawRenderTexture (RenderTexture2D rt, float a = 1) {
    Color tint(255,255,255,a * 255.);
    DrawTextureRec(rt.texture, {0,0,wSize.x,-wSize.y}, {0,0}, tint);
}

int main()
{
    // OscServer server(3333);

    InitWindow(wSize.x, wSize.y, "Osc Cpp");
    SetTargetFPS(FPS);
    ClearBackground(BLACK);

    Vector2 mouse {0,0};
    int mouseZ = 0;
    int frame = 0;

    _log("GetWorkingDirectory()", GetWorkingDirectory());

    const int feedbackCacheSize = 2;
    std::vector<RenderTexture2D> feedbackCache;
    for (int i = 0; i < feedbackCacheSize; i++) {
        RenderTexture2D target = LoadRenderTexture(wSize.x, wSize.y);
        // RenderTexture2D target = LoadRT32(wSize.x, wSize.y);
        feedbackCache.push_back(target);
    }

    Shader blurShader = LoadShader(0, "../resources/shaders/blur.frag.glsl");

    if (!IsShaderReady(blurShader))
        throw std::runtime_error("Shader not ready");

    // DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint)

    while (!WindowShouldClose()) {
        double now = GetTime(); frame++;
        mouse = GetMousePosition();
        mouseZ = IsMouseButtonPressed(0) ? 1 : (IsMouseButtonPressed(1) ? -1 : 0);

        BeginDrawing();
            ClearBackground(BLACK);

            // try {
            //     server.recv([](OSCPP::Server::Message msg) {
            //         OSCPP::Server::ArgStream args = msg.args();
            //         std::cout << "Received: " << msg << std::endl;
            //     });
            // } catch (const std::exception& e) {
            //     std::cerr << e.what() << std::endl;
            // }

            RenderTexture2D thisRT = feedbackCache[frame % feedbackCacheSize];
            RenderTexture2D prevRT = feedbackCache[(frame+1) % feedbackCacheSize];

            BeginTextureMode(thisRT);
                DrawCircle(mouse.x, mouse.y, 20, WHITE);

                BeginShaderMode(blurShader);
                    DrawRenderTexture(prevRT, 0);
                EndShaderMode();
            EndTextureMode();

            DrawRenderTexture(thisRT);
            DrawText(TextFormat("%ims / %ifps", (int)((GetTime() - now) * 1000.), GetFPS()), 20, 20, 30, WHITE);
            // EndShaderMode();
            // DrawText(TextFormat("%i : %i", (int)mouse.x, (int)mouse.y), 20, 50, 30, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}