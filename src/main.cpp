#include <cstdio>
#include <iostream>
#include <cstring>
#include <vector>

#include "raylib.h"

#include <oscpp/print.hpp>
// #include "OscServer.hpp"

#include "log.hpp"
// #include "ogl.hpp"

float FPS = 60.f;
float FRAME_TIME = 1. / FPS;
Vector2 wSize {1280, 720};

void DrawRenderTexture (RenderTexture2D rt) {
    DrawTextureRec(rt.texture, {0,0,wSize.x,-wSize.y}, {0,0}, WHITE);
}

int main()
{
    // OscServer server(3333);
    InitWindow(wSize.x, wSize.y, "Osc Cpp");
    wSize = {(float)GetMonitorWidth(0), (float)GetMonitorHeight(0)};
    printf("WINDOW SIZE: %f - %f", wSize.x, wSize.y);
    ToggleBorderlessWindowed();
    SetWindowSize(wSize.x, wSize.y);
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
        // BeginDrawing();
        //     BeginTextureMode(target);
        //         ClearBackground(BLACK);
        //     EndTextureMode();
        // EndDrawing();
        feedbackCache.push_back(target);
    }

    Shader blurShader = LoadShader(0, "../resources/shaders/blur.frag.glsl");
    if (!IsShaderReady(blurShader))
        throw std::runtime_error("blur Shader not ready");
    Shader edgeShader = LoadShader(0, "../resources/shaders/edge.frag.glsl");
    if (!IsShaderReady(edgeShader))
        throw std::runtime_error("edge Shader not ready");
    Shader opacityShader = LoadShader(0, "../resources/shaders/opacity.frag.glsl");
    if (!IsShaderReady(opacityShader))
        throw std::runtime_error("opacity Shader not ready");

    // DrawTextureRec(Texture2D texture, Rectangle source, Vector2 position, Color tint)

    Vector2& pointer = mouse;

    double frameTime = 0.;
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
                // Color circleColor = ColorFromHSV(now * 30., 1., 1.);
                Color circleColor = {255,0,0,255};
                DrawCircle(pointer.x, pointer.y, 40, circleColor);

                BeginShaderMode(edgeShader);
                    DrawRenderTexture(prevRT);
                EndShaderMode();
                BeginShaderMode(blurShader);
                    DrawRenderTexture(thisRT);
                EndShaderMode();
                BeginShaderMode(opacityShader);
                    DrawRenderTexture(thisRT);
                EndShaderMode();
            EndTextureMode();

            DrawRenderTexture(thisRT);
            // EndShaderMode();
            DrawText(TextFormat("%i fps", GetFPS()), 20, 20, 30, WHITE);
            // DrawText(TextFormat("%i : %i", (int)mouse.x, (int)mouse.y), 20, 50, 30, BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}