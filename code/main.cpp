#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <adGL.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>

#define VSYNC 0

struct GLVersion
{
    int Major;
    int Minor;
} gl_version;

uint32_t WND_WIDTH  = 920;
uint32_t WND_HEIGHT = 680;

bool g_running = true;

Controls controls = {};

const uint64_t MAX_DT_SAMPLES = 1;

double dt_samples[MAX_DT_SAMPLES] = {};
double dt_averaged                = 0;

// #ifdef _WIN32

// #if defined(NO_CONSOLE) && defined(_WIN32)
// int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
// {
//     return SDL_main(__argc, __argv);
// }
// #else
extern int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_Log("Video initialization failed: %s\n", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    int           window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    SDL_Window   *window       = SDL_CreateWindow("awesome", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WND_WIDTH, WND_HEIGHT, window_flags);
    SDL_GLContext glcontext    = SDL_GL_CreateContext(window);

    if (SDL_GL_SetSwapInterval(VSYNC) < 0) {
        SDL_Log("not supported!\n");
    }

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return -1;
    }

#if defined(_DEBUG)
    SDL_Log("--Debug mode--");
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
#endif

    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_version.Major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_version.Minor);
    SDL_Log("Initialized OpenGL context %d.%d Core Profile", gl_version.Major, gl_version.Minor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    GLInit();


    while (g_running) {
        uint64_t start = SDL_GetPerformanceCounter();

        float startTicks = SDL_GetTicks();

        SDL_Event event = {};
        while (SDL_PollEvent(&event)) {
            SDL_Keycode key = event.key.keysym.sym;
            switch (event.type) {

                // https://wiki.libsdl.org/SDL_Event
                //////////////////////////////
                // Key Events
                case SDL_KEYUP:
                    if (key == SDLK_ESCAPE) g_running = false;
                    if (key == SDLK_UP || key == SDLK_w) controls.up = false;
                    if (key == SDLK_DOWN || key == SDLK_s) controls.down = false;
                    if (key == SDLK_LEFT || key == SDLK_a) controls.left = false;
                    if (key == SDLK_RIGHT || key == SDLK_d) controls.right = false;
                    break;

                case SDL_KEYDOWN:
                    if (key == SDLK_UP || key == SDLK_w) controls.up = true;
                    if (key == SDLK_DOWN || key == SDLK_s) controls.down = true;
                    if (key == SDLK_LEFT || key == SDLK_a) controls.left = true;
                    if (key == SDLK_RIGHT || key == SDLK_d) controls.right = true;
                    break;
                // END Key Events

                ///////////////////////////////
                // WIndow Events
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                            g_running = false;
                            break;

                        default:
                            break;
                    }
                // END Window Events
                default:
                    break;
            }
        }

        static double acc = 0;
        if ((acc += dt_averaged) > 1) {
            SDL_SetWindowTitle(window, std::to_string(1 / dt_averaged).c_str());
            acc = 0;
        }

        GLUpdateAndRender(dt_averaged);
        SDL_GL_SwapWindow(window);

        uint64_t end = SDL_GetPerformanceCounter();

        static uint64_t idx                = 0;
        dt_samples[idx++ % MAX_DT_SAMPLES] = (end - start) / (double)SDL_GetPerformanceFrequency();

        double sum = 0;
        for (uint64_t i = 0; i < MAX_DT_SAMPLES; i++) {
            sum += dt_samples[i];
        }

        dt_averaged = sum / MAX_DT_SAMPLES;
    }

    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);

    return 0;
}
// #endif

// #endif