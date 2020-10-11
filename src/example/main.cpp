#include <iostream>
#include <SDL_image.h>
#include <SDL.h>
#include <imgui.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>

#include "textures.h"
#include "renderer.h"

int main(int argc, char *argv[])
{
    SDL_Window *wnd = SDL_CreateWindow(
        "Texture Atlas Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!wnd)
    {
        std::cerr << "SDL_GLContext failed: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //Doesn't seem to work on Windows 10, RX5700 xt. Instead, an offscreen 
    //rendering buffer is created on the Renderer::Init routine.
    //TODO:: Maybe investigate why is this a thing.
    //Clues:: RenderDoc claims "1 sample"
#if 0    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
#endif
    SDL_GLContext ctx = SDL_GL_CreateContext(wnd);
    if (!ctx)
    {
        std::cerr << "SDL_GLContext failed: " << SDL_GetError() << "\n";
        return -1;
    }

    if (gladLoadGL() == 0)
    {
        std::cerr << "GLAD failed! Could not load OpenGL library.\n";
        return -1;
    }

    SDL_GL_MakeCurrent(wnd, ctx);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    //ImGui initialization routines
    ImGui_ImplSDL2_InitForOpenGL(wnd, ctx);
    ImGui_ImplOpenGL3_Init(NULL);

    // Sanity check output
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "OpenGL version: " << major << "." << minor << "\n";

    Textures::Init();
    Renderer::Init();
    Renderer::LoadScene("data/sponza/", "Sponza.gltf");

    int done = 0;
    do
    {
        SDL_Event ev;
        while (SDL_PollEvent(&ev))
        {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (ev.type == SDL_QUIT)
                done = 1;
            if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE && ev.window.windowID == SDL_GetWindowID(wnd))
                done = 1;
        }

        //Start a new ImGui Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(wnd);
        ImGui::NewFrame();

        //Mesh drawing routines
        Renderer::Render((int)io.DisplaySize.x, (int)io.DisplaySize.y);
        Textures::ImGui();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(wnd);
    } while (!done);

    Textures::Destroy();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(wnd);
    SDL_Quit();
    return 0;
}