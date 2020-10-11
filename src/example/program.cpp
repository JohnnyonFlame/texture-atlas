#include <utility>
#include <tuple>
#include <vector>
#include <map>
#include <array>
#include <string>
#include <fstream>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "program.h"

static std::string get_shader_contents(const std::string &filename)
{
    std::ifstream ifs(filename);
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
}

static void shader_print_errlog(GLuint shader)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLchar err[2048];
        GLint max_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_len);
        glGetShaderInfoLog(shader, max_len, &max_len, err);
        fprintf(stderr, "Failed to compile shader.\n%s\n", err);
    }
}

static void program_print_errlog(GLuint program)
{
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLchar err[2048];
        GLint max_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len);
        glGetProgramInfoLog(program, max_len, &max_len, err);
        fprintf(stderr, "Failed to link shader program.\n%s\n", err);
    }
}

Program::Program(const std::string &vert, const std::string &frag,
                 const AttributeList &attrs, const UniformList &unifs)
    : uniforms(*this), attributes(*this)
{
    std::string vert_source(get_shader_contents(vert));
    std::string frag_source(get_shader_contents(frag));

    GLint vert_len = (GLint)vert_source.length(),
          frag_len = (GLint)frag_source.length();
    GLchar *vert_source_cstr = (GLchar *)vert_source.c_str(),
           *frag_source_cstr = (GLchar *)frag_source.c_str();

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_shader, 1, &vert_source_cstr, &vert_len);
    glCompileShader(vert_shader);
    shader_print_errlog(vert_shader);

    glShaderSource(frag_shader, 1, &frag_source_cstr, &frag_len);
    glCompileShader(frag_shader);
    shader_print_errlog(frag_shader);

    program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    program_print_errlog(program);

    for (auto &uniform : unifs)
    {
        this->uniform_map[uniform] = uniform;
        this->uniform_map[uniform].loc =
            glGetUniformLocation(this->program, uniform.c_str());
    }

    GLuint i = 1;
    for (auto attrib : attrs)
    {
        this->attribute_map[attrib] = glGetAttribLocation(program, attrib.c_str());
        std::cout << "Bound " << attrib << " to " << i << "\n";
        i++;
    }
}

void Program::Active()
{
    glUseProgram(this->program);
}

//Initialize from a string (identifier name)
Uniform &Uniform::operator=(const std::string &other)
{
    this->bad = false;
    this->identifier = other;
    return *this;
}

Uniform &Program::Uniforms::operator[](const std::string &idx)
{
    //Check if we have the uniform
    auto got = prog.uniform_map.find(idx);

    //If not, return a static "bad uniform" type that does nothing
    if (got == prog.uniform_map.end())
    {
        //Don't spam bad uniform warnings.
        auto already_warned = prog.bad_uniform_warned.find(idx);
        if (already_warned == prog.bad_uniform_warned.end())
        {
            std::cerr << "Warning: Uniform '" << idx << "' not found.\n";
            prog.bad_uniform_warned[idx] = true;
        }

        return Program::bad;
    }

    //Otherwise return the real deal
    return got->second;
}

Uniform &Uniform::operator<<(const float &other)
{
    glUniform1f(this->loc, other);
    return *this;
}

GLuint Program::Attributes::operator[](const std::string &idx)
{
    auto got = prog.attribute_map.find(idx);
    if (got == prog.attribute_map.end())
    {
        std::cerr << "Unknown vertex attribute '" << idx << "'!\n";
        return (GLuint)-1;
    }

    return got->second;
}

/* Internal Usage */
Uniform Program::bad;
bool Uniform::is_bad()
{
    return this->bad;
}