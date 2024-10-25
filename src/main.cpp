#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include <oscpp/print.hpp>
#include "OscServer.hpp"

#include "StateChangeDetector.hpp"
#include "config.hpp"

Config CONF;
float FPS = 60.f;
float FRAME_TIME = 1. / FPS;
Vector2 wSize {1280, 720};

float mapRange(float x, float in_min, float in_max, float out_min, float out_max) {
    return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
}

float lag(float current, float target, float smoothingFactor, float dt) {
    return current + (target - current) * smoothingFactor * dt;
}

void DrawRenderTexture (RenderTexture2D rt) {
    DrawTextureRec(rt.texture, {0,0,wSize.x,-wSize.y}, {0,0}, WHITE);
}

class Pointer {
public:
    Vector2 pos {0, 0};
    float speed = 0.;
    Vector2 _targetPos;

    Pointer (Vector2 pos) : pos(pos) {}

    void update (float dt, Vector2 targetPos) {
        _targetPos = targetPos;
        Vector2 nextPos {
            lag(pos.x, targetPos.x, 3., dt),
            lag(pos.y, targetPos.y, 3., dt) };

        Vector2 diff = Vector2Subtract(nextPos, pos);
        speed = Vector2Length(diff);

        pos.x = nextPos.x;
        pos.y = nextPos.y;
    }
};


int main()
{
    CONF = LoadConfig();

    OscServer server(CONF.port);
    InitWindow(wSize.x, wSize.y, "Osc Cpp");
    ToggleBorderlessWindowed();
    HideCursor();
    DisableCursor();
    wSize = {(float)GetMonitorWidth(0), (float)GetMonitorHeight(0)};
    printf("WINDOW SIZE: %f - %f", wSize.x, wSize.y);
    SetWindowSize(wSize.x, wSize.y);

    printf("JSON: %s\n", CONF.pointerX_chan.c_str());

    // SetTargetFPS(FPS);
    ClearBackground(BLACK);

    Vector2 mouse {wSize.x/2,wSize.y/2};
    int mouseZ = 0;
    int frame = 0;

    // _log("GetWorkingDirectory()", GetWorkingDirectory());

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

    Pointer pointer ({ wSize.x/2, wSize.y/2 });
    StateChangeDetector fullscreenBtn;
    StateChangeDetector reloadConfigBtn;
    StateChangeDetector mouseModeBtn;
    bool mouseMode;
    StateChangeDetector showDebugBtn;
    bool showDebug;
    bool oscGotMessage = false;

    std::unordered_map<std::string, float> oscChanVals;

    double frameTime = 0.;
    while (!WindowShouldClose()) {
        double now = GetTime(); frame++;
        double delta = GetFrameTime();
        if (mouseMode) {
            mouse = GetMousePosition();
            mouseZ = IsMouseButtonPressed(0) ? 1 : (IsMouseButtonPressed(1) ? -1 : 0);
        }

        if (fullscreenBtn.hasChangedOn(IsKeyPressed(KEY_F)))
            ToggleBorderlessWindowed();
        if (mouseModeBtn.hasChangedOn(IsKeyPressed(KEY_M)))
            mouseMode = !mouseMode;
        if (showDebugBtn.hasChangedOn(IsKeyPressed(KEY_D)))
            showDebug = !showDebug;
        if (reloadConfigBtn.hasChangedOn(IsKeyPressed(KEY_R)))
            CONF = LoadConfig();

        BeginDrawing();
            ClearBackground(BLACK);

            // try {
                server.recv([&oscChanVals, &oscGotMessage](OSCPP::Server::Message msg) {
                    oscGotMessage = true;
                    OSCPP::Server::ArgStream args = msg.args();
                    std::string address = msg.address();

                    oscChanVals[address] = args.float32();
                });
            // } catch (const std::exception& e) {
            //     std::cerr << e.what() << std::endl;
            // }
            Vector2 pointerTarget =  (mouseMode || !oscGotMessage) ? mouse
                : (Vector2) {
                    mapRange(oscChanVals[CONF.pointerX_chan], CONF.pointerX_range[0], CONF.pointerX_range[1], 0., wSize.x),
                    mapRange(oscChanVals[CONF.pointerY_chan], CONF.pointerY_range[0], CONF.pointerY_range[1], wSize.y, 0.),
                };
            pointer.update(delta,pointerTarget);

            RenderTexture2D thisRT = feedbackCache[frame % feedbackCacheSize];
            RenderTexture2D prevRT = feedbackCache[(frame+1) % feedbackCacheSize];

            BeginTextureMode(thisRT);
                Color circleColor = {255,150,0,255};
                float radius = (!mouseMode && !oscGotMessage) ? (!((int)(now*2) % 10) ? 40 : 0)
                    : Clamp(mapRange(pointer.speed, 0, 18, 0, 40), 0, 80);
                DrawCircle(
                    pointer.pos.x, pointer.pos.y,
                    radius, circleColor);

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

            DrawText(TextFormat("%i fps", GetFPS()), 20, 20, 30, WHITE);
            if (showDebug) {
                DrawText(TextFormat("pointer: %.0f : %.0f", pointer._targetPos.x, pointer._targetPos.y), 20, 50, 30, WHITE);
                DrawText(TextFormat("speed: %.3f", pointer.speed), 20, 80, 30, WHITE);
            }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}