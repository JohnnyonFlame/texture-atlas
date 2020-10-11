#ifndef __EXAMPLE_ATLAS_H__
#define __EXAMPLE_ATLAS_H__

namespace Textures
{
    int Init();
    void Destroy();
    int Load(const std::string &path, const std::string &name);
    GLuint Lookup(const std::string &name);
    GLuint LookupVirtual(const std::string &name);
    void RenderImGUI();
    glm::vec4 VirtualCoords(GLuint vtex_id);
    void ImGui();
}; // namespace Textures

#endif /* __EXAMPLE_ATLAS_H__ */