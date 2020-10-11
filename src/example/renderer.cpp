#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "textures.h"
#include "program.h"
#include "mesh.h"
#include "renderer.h"

static Program *prog = NULL;
static const aiScene *scene = NULL;
Assimp::Importer importer;
static std::vector<Mesh> meshes;

static glm::vec3 eye(2, 2, 0), center(0, 1, 0), up(0, 1, 0);
static glm::vec3 sun = glm::normalize(glm::vec3(0.07f, -0.85f, 0.53f));

GLuint fbaa, fbd, fbo;

void Renderer::Init()
{
    if (prog)
        return;

    prog = new Program("data/shaders/tex.vert", "data/shaders/tex.frag",
                       /* Attributes */
                       {{"aPosition"},
                        {"aNormal"},
                        {"aTangent"},
                        {"aTextureCoords"}},
                       /* Uniforms */
                       {{"uMVPMat"},
                        {"uMVMat_trinv"},
                        {"uTexture0"},
                        {"uTexture1"},
                        {"uDiffuseDirection"}});

    glGenTextures(1, &fbaa);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbaa);

    glGenTextures(1, &fbd);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbd);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fbaa, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, fbd, 0);
}

void Renderer::Render(int w, int h)
{
    static float prev_w = 0, prev_h = 0;

    if (w != prev_w || h != prev_h)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbaa);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, w, h, GL_TRUE);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbd);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH24_STENCIL8, w, h, GL_TRUE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_DEPTH_TEST);
    glDepthRange(0.01f, 10000.f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    //glDisable(GL_CULL_FACE);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glEnable(GL_SAMPLE_ALPHA_TO_ONE);

    glm::mat4 model = glm::identity<glm::mat4>();
    glm::mat4 view = glm::lookAt(eye, center, up);
    if (w <= 0 || h <= 0)
        return;
    
    glm::mat4 proj = glm::perspective(glm::radians(64.0f), (float)w / (float)h, 0.1f, 1000.f);
    sun = glm::normalize(sun);
    if (sun.x == INFINITY || sun.y == INFINITY || sun.z == INFINITY)
        sun = glm::vec3(0.0f, 1.0f, 0.0f);

    ImGui::Begin("Camera");
    ImGui::InputFloat3("eye", glm::value_ptr(eye), 2);
    ImGui::InputFloat3("center", glm::value_ptr(center), 2);
    ImGui::InputFloat3("up", glm::value_ptr(up), 2);
    ImGui::InputFloat3("sun", glm::value_ptr(sun), 2);
    ImGui::End();

    Textures::RenderImGUI();

    glm::mat4 mvp = proj * view * model;
    glm::mat4 mv_trinv = glm::transpose(glm::inverse(view * model));

    prog->Active();
    prog->uniforms["uMVPMat"] << mvp;
    prog->uniforms["uMVMat_trinv"] << mv_trinv;
    prog->uniforms["uDiffuseDirection"] << sun;
    prog->uniforms["uTexture0"] << 0;
    prog->uniforms["uTexture1"] << 1;

    for (auto &mesh : meshes)
        mesh.Render(model, view, proj, sun);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);   // Make sure no FBO is set as the draw framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo); // Make sure your multisampled FBO is the read framebuffer
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

static void LoadTextures(const std::string &path, aiMaterial *mat, aiTextureType type)
{
    unsigned int textureCount = mat->GetTextureCount(type);
    for (unsigned int j = 0; j < textureCount; j++)
    {
        aiString texture_path;
        mat->GetTexture(type, j, &texture_path);
        if (texture_path.length < 2)
            continue;

        Textures::Load(path, texture_path.C_Str());
    }
}

static GLuint GetTextureType(aiMaterial *mat, aiTextureType type)
{
    aiString texture_path;
    if (mat->GetTextureCount(type) > 0)
    {
        mat->GetTexture(type, 0, &texture_path);
        return Textures::Lookup(texture_path.C_Str());
    }

    return -1;
}

int Renderer::LoadScene(const std::string &path, const std::string &file)
{
    std::string fullpath = path + file;
    if (scene)
    {
        Textures::Destroy();
        Textures::Init();

        importer.FreeScene();
    }

    std::cerr << "Loading " << file << "\n";
    scene = importer.ReadFile(fullpath.c_str(),
                              aiProcessPreset_TargetRealtime_Fast |
                                  aiProcess_Triangulate |
                                  aiProcess_FlipUVs |
                                  aiProcess_CalcTangentSpace |
                                  aiProcess_ConvertToLeftHanded |
                                  aiProcess_PreTransformVertices);
    std::cerr << "Loaded " << file << "\n";
    //Load all scene textures
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        aiMaterial *mat = scene->mMaterials[i];
        LoadTextures(path, mat, aiTextureType_DIFFUSE);
        LoadTextures(path, mat, aiTextureType_NORMALS);
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMaterial *mat = scene->mMaterials[scene->mMeshes[i]->mMaterialIndex];
        GLuint diffuse = GetTextureType(mat, aiTextureType_DIFFUSE),
               normals = GetTextureType(mat, aiTextureType_NORMALS);
        meshes.emplace_back(scene->mMeshes[i], *prog, diffuse, normals);
    }

    return 1;
}