#include <vector>
#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "program.h"
#include "mesh.h"

Mesh::Mesh(aiMesh *mesh, Program &prog, GLuint diffuse, GLuint normals)
{
    this->tris = mesh->mNumFaces;

    //each face has: (x, y, z), (nx, ny, nz), (u, v)
    this->vertex_data.resize(this->tris * 3 * (3 + 3 + 3 + 2));

    unsigned int mesh_idx = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace *face = &mesh->mFaces[i];

        for (int j = 0; j < 3; j++)
        {
            aiVector3D &vertex = mesh->mVertices[face->mIndices[j]];
            aiVector3D &normal = mesh->mNormals[face->mIndices[j]];
            aiVector3D &tangents = mesh->mTangents[face->mIndices[j]];
            aiVector3D &uv = mesh->mTextureCoords[0][face->mIndices[j]];

            this->vertex_data[mesh_idx++] = vertex.x;
            this->vertex_data[mesh_idx++] = vertex.y;
            this->vertex_data[mesh_idx++] = vertex.z;

            this->vertex_data[mesh_idx++] = normal.x;
            this->vertex_data[mesh_idx++] = normal.y;
            this->vertex_data[mesh_idx++] = normal.z;

            this->vertex_data[mesh_idx++] = tangents.x;
            this->vertex_data[mesh_idx++] = tangents.y;
            this->vertex_data[mesh_idx++] = tangents.z;

            this->vertex_data[mesh_idx++] = uv.x;
            this->vertex_data[mesh_idx++] = uv.y;
        }
    }

    this->diffuse = diffuse;
    this->normals = normals;
    
    prog.Active();
    glGenVertexArrays(1, &this->vao);
    glBindVertexArray(this->vao);

    glGenBuffers(1, &this->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

    GLuint pos = prog.attributes["aPosition"],
           norm = prog.attributes["aNormal"],
           tang = prog.attributes["aTangent"],
           tex = prog.attributes["aTextureCoords"];
    
    glEnableVertexAttribArray(pos);
    glEnableVertexAttribArray(norm);
    glEnableVertexAttribArray(tang);
    glEnableVertexAttribArray(tex);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(0 * sizeof(float)));
    glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(tang, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));

    glBufferData(GL_ARRAY_BUFFER, this->vertex_data.size() * sizeof(float), (void *)&this->vertex_data[0], GL_STREAM_DRAW);
}

void Mesh::Render(glm::mat4 model, glm::mat4 view, glm::mat4 proj, glm::vec3 sun)
{
    glBindVertexArray(this->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->diffuse);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->normals);
    glDrawArrays(GL_TRIANGLES, 0, this->tris * 3);
}