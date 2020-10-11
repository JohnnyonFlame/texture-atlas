#include <iostream>
#include <map>
#include <vector>
#include <fstream>

#include <SDL.h>
#include <SDL_image.h>
#include <imgui.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "misc.h"
#include "texture_atlas.h"
#include "textures.h"

/**
 * Texture Atlas page and accompanying partitioning structure
 */
GLuint tex_page;
static Atlas *atlas = NULL;
static std::map<std::string, GLuint> textures;
static std::map<std::string, GLuint> vtextures;

/**
 * OpenGL texture setup routine.
 */
static GLuint texture_init(GLenum filter, unsigned int size = 0)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    //Texture parameters setup
    glTexParameterf(GL_TEXTURE_2D, 0x84FE, 16);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (filter == GL_LINEAR) ? GL_LINEAR_MIPMAP_LINEAR : filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //Initialize empty texture page when we have a size argument
    if (size)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    return tex;
}

int Textures::Init()
{
    if (atlas)
        return 1;

    // Create a texture atlas
    if (!atlas_create(&atlas, 8096, 16))
    {
        std::cerr << "Atlas creation failed.\n";
        return 0;
    }

    // Create the corresponding texture page
    tex_page = texture_init(GL_NEAREST, atlas_get_dimensions(atlas));

    return 1;
}

void Textures::Destroy()
{
    //Delete created OpenGL textures
    for (auto texture : textures)
        glDeleteTextures(1, &texture.second);
    textures.clear();

    //Delete created OpenGL vtextures
    glDeleteTextures(1, &tex_page);
    atlas_destroy(atlas);
    atlas = NULL;
    vtextures.clear();
}

#define FREE_ON_EXIT(x) auto free_##x = finally([&x] { if(x) SDL_FreeSurface(x); })

static SDL_Surface *loadAsRGBA32(const std::string &fullpath)
{
    //Load original texture
    SDL_Surface *img = IMG_Load(fullpath.c_str());
    FREE_ON_EXIT(img);

    if (!img)
    {
        std::cerr << "Failed to load '" << fullpath.c_str() << "'.\n";
        return NULL;
    }

    //Convert into a new one and free the original one
    SDL_Surface *img_conv = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    if (!img_conv)
    {
        std::cerr << "Failed to convert image '" << fullpath.c_str() << ".\n";
        return NULL;
    }

    return img_conv;
}

int Textures::Load(const std::string &path, const std::string &file)
{
    //Try and find if we loaded this filename before
    auto id = textures.find(file);

    //Texture not found, let's allocate it.
    if (id == textures.end())
    {
        std::cerr << "Attempting to load " << file << ".\n";

        //Load texture as RGBA32
        SDL_Surface *tex = loadAsRGBA32(path + file);
        FREE_ON_EXIT(tex);
        if (!tex)
            return 0;

        //Create a vanilla OpenGL texture
        GLuint tex_id = texture_init(GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);
        glGenerateMipmap(GL_TEXTURE_2D);

        //Create an atlased OpenGL texture
        //Reserve a texture id for ourselves
        GLuint vtex_id;
        atlas_gen_texture(atlas, &vtex_id);

        //Allocate space for it somewhere in the atlas
        if (atlas_allocate_vtex_space(atlas, vtex_id, tex->w, tex->h))
        {
            uint16_t xywh[4];
            atlas_get_vtex_xywh_coords(atlas, vtex_id, 0, &xywh[0]);

            //Now upload the texture
            glBindTexture(GL_TEXTURE_2D, tex_page);
            glTexSubImage2D(GL_TEXTURE_2D, 0, xywh[0], xywh[1], xywh[2], xywh[3],
                            GL_RGBA, GL_UNSIGNED_BYTE, tex->pixels);
        }
        else
        {
            std::cerr << "Failed to allocate atlas space for '" << file.c_str() << "'.\n";
            return 0;
        }

        textures[file] = tex_id;
        vtextures[file] = vtex_id;
    }

    return 1;
}

/**
 * Generic texture lookup.
 */
template <class T>
static unsigned int lookup_tex(const T &container, const std::string &name)
{
    auto tex = container.find(name);
    if (tex != container.end())
        return tex->second;
    else
        return -1;
}

GLuint Textures::Lookup(const std::string &name)
{
    return lookup_tex(textures, name);
}

GLuint Textures::LookupVirtual(const std::string &name)
{
    return lookup_tex(vtextures, name);
}

#include "imgui_internal.h"
void ScrollWhenDraggingOnVoid(const ImVec2& delta)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;
    bool hovered = false;
    bool held = false;
    if (g.HoveredId == 0) // If nothing hovered so far in the frame (not same as IsAnyItemHovered()!)
        ImGui::ButtonBehavior(window->Rect(), window->GetID("##scrolldraggingoverlay"), &hovered, &held, ImGuiButtonFlags_MouseButtonLeft);
    if (held)
    {
        window->Scroll.x += delta.x;
        window->Scroll.y += delta.y;
    }
}

void Textures::RenderImGUI()
{
    float tex_size = (float)atlas_get_dimensions(atlas);

    ImGui::Begin("Atlas", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    float wheel_delta = ImGui::GetIO().MouseWheel;
    static float scale = 1.f;
    scale += 0.05f * wheel_delta;
    ImGui::Image((void*)(intptr_t)tex_page, ImVec2(tex_size * powf(scale, 3), tex_size * powf(scale, 3)));
    
    ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;
    ScrollWhenDraggingOnVoid(ImVec2(-mouse_delta.x, -mouse_delta.y));
    ImGui::End();
}

glm::vec4 Textures::VirtualCoords(GLuint vtex_id)
{
    float uvst[4];
    if (atlas_get_vtex_uvst_coords(atlas, vtex_id, 0, &uvst[0]))
        return glm::vec4(uvst[0], uvst[1], uvst[2], uvst[3]);
    else
        return glm::vec4(0, 0, 0, 0);
}

void Textures::ImGui()
{
    //STUB
    //TODO:: unstub
}