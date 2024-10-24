#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#include <oscpp/print.hpp>
#include "OscServer.hpp"

#include "log.hpp"
// #include "ogl.hpp"

float FPS = 60.f;
float FRAME_TIME = 1. / FPS;
Vector2 wSize {1280, 720};

void DrawRenderTexture (RenderTexture2D rt) {
    DrawTextureRec(rt.texture, {0,0,wSize.x,-wSize.y}, {0,0}, WHITE);
}

float mapRange(float x, float in_min, float in_max, float out_min, float out_max) {
    return out_min + (x - in_min) * (out_max - out_min) / (in_max - in_min);
}

float lag(float current, float target, float smoothingFactor, float dt) {
    return current + (target - current) * smoothingFactor * dt;
}

class Pointer {
public:
    Vector2 pos {0, 0};
    float speed = 0.;

    void update (float dt, Vector2 targetPos) {
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
    OscServer server(3337);
    InitWindow(wSize.x, wSize.y, "Osc Cpp");
    wSize = {(float)GetMonitorWidth(0), (float)GetMonitorHeight(0)};
    printf("WINDOW SIZE: %f - %f", wSize.x, wSize.y);
    // ToggleBorderlessWindowed();
    SetWindowSize(wSize.x, wSize.y);
    // SetTargetFPS(FPS);
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

    Pointer pointer;

    // using FuncType = std::function<void(float)>;
    // std::unordered_map<std::string, FuncType> chanToFunc {
    //     {"/leftWrist_x", [&pointer](float val) { pointer.x = val; }},
    //     {"/leftWrist_y", [&pointer](float val) { pointer.y = val; }},
    // };

    std::unordered_map<std::string, float> oscChanVals;

    double frameTime = 0.;
    while (!WindowShouldClose()) {
        double now = GetTime(); frame++;
        double delta = GetFrameTime();
        mouse = GetMousePosition();
        mouseZ = IsMouseButtonPressed(0) ? 1 : (IsMouseButtonPressed(1) ? -1 : 0);

        BeginDrawing();
            ClearBackground(BLACK);

            // try {
                server.recv([&oscChanVals](OSCPP::Server::Message msg) {
                    OSCPP::Server::ArgStream args = msg.args();
                    std::string address = msg.address();
                    // float v = args.float32();
                    // if (msg == "/leftWrist_x")
                    //     printf("Received: %s %f\n", address.c_str(), v);
                    // print()
                    oscChanVals[address] = args.float32();
                    // auto it = chanToFunc.find(address);
                    // if (it != chanToFunc.end()) {
                    //     FuncType func = it->second;
                    //     func(args.float32());
                    // } else if (true)
                        // std::cout << "Received: " << msg << std::endl;
                });
            // } catch (const std::exception& e) {
            //     std::cerr << e.what() << std::endl;
            // }

            pointer.update(delta,
                (Vector2) { oscChanVals["/leftWrist_x"], oscChanVals["/leftWrist_y"] });
            printf("speed: %f\n", pointer.speed);
            // printf("Pointer: %f, %f\n", pointer.pos.x, pointer.pos.y);
            // printf("Pointer.target: %f, %f\n", pointer.targetPos.x, pointer.targetPos.y);

            RenderTexture2D thisRT = feedbackCache[frame % feedbackCacheSize];
            RenderTexture2D prevRT = feedbackCache[(frame+1) % feedbackCacheSize];

            BeginTextureMode(thisRT);
                // Color circleColor = ColorFromHSV(now * 30., 1., 1.);
                Color circleColor = {255,0,0,255};
                DrawCircle(
                    mapRange(pointer.pos.x, 0.5, 1.5, 0., wSize.x),
                    mapRange(pointer.pos.y, 1., 3., wSize.y, 0.),
                    Clamp(mapRange(pointer.speed * 100., 0, 2, 0, 40), 0, 80),
                    circleColor);

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