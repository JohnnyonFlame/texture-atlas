#ifndef __EXAMPLE_MESH_H__
#define __EXAMPLE_MESH_H__

class Mesh
{
    int tris;

public:
    Mesh(aiMesh *mesh, Program &prog, GLuint diffuse, GLuint normals);
    std::vector<GLfloat> vertex_data;
    GLuint diffuse;
    GLuint normals;
    GLuint vertex_data_buffer;
    GLuint vao;
    GLuint vbo;

    void Render(glm::mat4 model, glm::mat4 view, glm::mat4 proj, glm::vec3 sun);
};

#endif /* __EXAMPLE_MESH_H__ */