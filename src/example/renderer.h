#ifndef __RENDERER_H__
#define __RENDERER_H__

namespace Renderer
{
    void Init();
    void Render(int w, int h);
    int LoadScene(const std::string &path, const std::string &fullpath);
    void Destroy();
}; // namespace Renderer

#endif /* __RENDERER_H__ */