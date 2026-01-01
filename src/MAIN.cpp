#define MA_ENABLE_DECODERS
#define MINIAUDIO_IMPLEMENTATION
#define NOMINMAX

#include <vector>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <random>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDL.h>
#include "miniaudio.h"
#include "GameConfig.h"
#include "SpatialGrid.h"
#include "ThreadPool.h"
#include "AudioSystem.h"
#include "PhysicsSystem.h"
#include "Render.h"
#include "GameLogic.h"
#include "keyjob.h"
#include "Simulation.h"


int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;
float currentMusicEnergy = 0.0f;

std::vector<SDL_Color> rainbowColorLUT(RAINBOW_LUT_SIZE);
std::vector<Particle> particles;
std::vector<Particle> particle_buffer;
std::vector<Vector2D> forces;
std::vector<RainbowFragment> rainbowFragments;
std::vector<BrushParticle> brushParticles;
std::vector<float> density_buffer;
int density_buffer_width = 0;
int density_buffer_height = 0;

SynthSound blueSound;
SynthSound rainbowSound;
SynthSound explosionSound;

std::vector<SynthSound*> sounds_to_play;
std::mutex sounds_to_play_mutex;
SDL_AudioDeviceID audioDevice = 0;

thread_local std::minstd_rand rng_sampler;


struct GameContext {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    ThreadPool* pool = nullptr;
    SpatialGrid* grid = nullptr;
    GameTextures textures;

    // Miniaudio
    ma_engine engine;
    ma_sound background_music;


    bool running = true;
    bool mouseDown = false;
    int mx = 0, my = 0;
    bool brushMode = false;
    bool painting = false;
    float lastBrushX = SCREEN_WIDTH / 2.0f;
    float lastBrushY = SCREEN_HEIGHT / 2.0f;
    float lastVelX = 0.0f;
    float lastVelY = 0.0f;
    int brushEffectMode = 1;
    bool playerSunMode = false;
    float playerSunTimer = 0.0f;
    bool playerRainbow = false;
    float playerRainbowTimer = 0.0f;
    float playerJumpTimer = 0.0f;
    float meteorTimer = 0.0f;
    float nextMeteorInterval = 2.0f;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float avgVx = 0.0f;
    float avgVy = 0.0f;


    Uint32 frameStart = 0;
    int frameTime = 0;
    Uint32 fpsLastTime = 0;
    int fpsFrames = 0;
    float finalFPS = 0.0f;
    bool showFPS = false;
    bool silent = true;
    int TARGET_FPS = 90;
};


GameContext* g_ctx = nullptr;

void main_loop_iteration() {
    if (!g_ctx->running) {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    g_ctx->frameStart = SDL_GetTicks();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        handle_input_events(e, g_ctx->running, g_ctx->mouseDown, g_ctx->brushMode, g_ctx->painting,
            g_ctx->brushEffectMode, g_ctx->showFPS, g_ctx->silent, *g_ctx->pool,
            *g_ctx->grid, g_ctx->renderer, g_ctx->textures);
    }

    SDL_GetMouseState(&g_ctx->mx, &g_ctx->my);

    update_physics_simulation(g_ctx->brushMode, g_ctx->mx, g_ctx->my, g_ctx->mouseDown,
        g_ctx->playerSunMode, g_ctx->playerRainbow, g_ctx->centerX, g_ctx->centerY,
        g_ctx->avgVx, g_ctx->avgVy, g_ctx->playerRainbowTimer, g_ctx->playerJumpTimer,
        *g_ctx->grid, *g_ctx->pool);

    update_meteors(g_ctx->meteorTimer, g_ctx->nextMeteorInterval, g_ctx->silent, g_ctx->TARGET_FPS);

    update_brush_painting(g_ctx->brushMode && g_ctx->painting, g_ctx->brushEffectMode,
        g_ctx->centerX, g_ctx->centerY, g_ctx->lastBrushX, g_ctx->lastBrushY,
        g_ctx->lastVelX, g_ctx->lastVelY);

    update_brush_particles(g_ctx->brushMode, g_ctx->playerSunMode, g_ctx->playerSunTimer);
    update_rainbow_fragments();

    if (g_ctx->playerRainbow) {
        g_ctx->playerRainbowTimer -= 0.016f;
        g_ctx->playerJumpTimer -= 0.016f;
        if (g_ctx->playerRainbowTimer < 0) g_ctx->playerRainbow = false;
    }
    if (g_ctx->playerSunMode) {
        g_ctx->playerSunTimer -= 0.016f;
        if (g_ctx->playerSunTimer <= 0) g_ctx->playerSunMode = false;
    }

    g_ctx->fpsFrames++;
    if (SDL_GetTicks() - g_ctx->fpsLastTime >= 1000) {
        g_ctx->finalFPS = g_ctx->fpsFrames * 1000.0f / (SDL_GetTicks() - g_ctx->fpsLastTime);
        g_ctx->fpsFrames = 0;
        g_ctx->fpsLastTime = SDL_GetTicks();
    }

    render_frame(g_ctx->renderer, g_ctx->textures, g_ctx->brushMode, g_ctx->brushEffectMode,
        g_ctx->playerSunMode, g_ctx->playerRainbow, g_ctx->playerJumpTimer,
        g_ctx->showFPS, g_ctx->finalFPS);

    SDL_RenderPresent(g_ctx->renderer);

#ifndef __EMSCRIPTEN__
    int frameTime = SDL_GetTicks() - g_ctx->frameStart;
    int delay = (1000 / g_ctx->TARGET_FPS) - frameTime;
    if (delay > 0) SDL_Delay(delay);
#endif
}

int main(int argc, char* argv[]) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    g_ctx = new GameContext();

    unsigned int n_threads = std::thread::hardware_concurrency();
    if (n_threads == 0) n_threads = 4;
    g_ctx->pool = new ThreadPool(n_threads);
    printf("Using %u threads.\n", n_threads);

    SDL_AudioSpec want = {}, have = {};
    want.freq = 44100; want.format = AUDIO_F32SYS; want.channels = 1; want.samples = 1024; want.callback = audio_callback;
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    make_blue_sound(blueSound); make_rainbow_sound(rainbowSound); make_stellar_explosion_sound(explosionSound);
    SDL_PauseAudioDevice(audioDevice, 0);

    g_ctx->window = SDL_CreateWindow("Gift From Other planet", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    g_ctx->renderer = SDL_CreateRenderer(g_ctx->window, -1, SDL_RENDERER_ACCELERATED);

    ma_result result;
    ma_engine_init(NULL, &g_ctx->engine);
    ma_sound_init_from_file(&g_ctx->engine, "Stellardrone - Eternity.mp3", MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_STREAM, NULL, NULL, &g_ctx->background_music);
    ma_sound_set_looping(&g_ctx->background_music, MA_TRUE);
    ma_sound_start(&g_ctx->background_music);

    srand((unsigned int)time(0));
    generateRainbowLUT();

    recreate_all_textures(g_ctx->renderer, g_ctx->textures, SCREEN_WIDTH, SCREEN_HEIGHT);

    density_buffer_width = SCREEN_WIDTH / DENSITY_BUFFER_SCALE;
    density_buffer_height = SCREEN_HEIGHT / DENSITY_BUFFER_SCALE;
    density_buffer.resize(density_buffer_width * density_buffer_height);

    g_ctx->grid = new SpatialGrid(SCREEN_WIDTH, SCREEN_HEIGHT, INTERACTION_RADIUS);

    particles.reserve(TOTAL_PARTICLES);
    forces.resize(TOTAL_PARTICLES);
    int global_index = 0;
    for (int i = 0; i < PLAYER_PARTICLE_COUNT; ++i) {
        float angle = (float)i / PLAYER_PARTICLE_COUNT * 2.0f * 3.14159f;
        float r = (rand() % 40);
        particles.emplace_back(SCREEN_WIDTH / 2 + cos(angle) * r, SCREEN_HEIGHT / 2 + sin(angle) * r, true);
        particles.back().id = global_index++;
    }
    for (int i = 0; i < TOTAL_PARTICLES - PLAYER_PARTICLE_COUNT; ++i) {
        particles.emplace_back(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, false);
        particles.back().id = global_index++;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop_iteration, 0, 1);
#else
    while (g_ctx->running) {
        main_loop_iteration();
    }
#endif
    destroy_all_textures(g_ctx->textures);
    SDL_DestroyRenderer(g_ctx->renderer);
    SDL_DestroyWindow(g_ctx->window);
    SDL_CloseAudioDevice(audioDevice);
    ma_sound_uninit(&g_ctx->background_music);
    ma_engine_uninit(&g_ctx->engine);
    delete g_ctx->pool;
    delete g_ctx->grid;
    delete g_ctx;
    SDL_Quit();
    return 0;
}