#ifndef __EXAMPLE_PROGRAM_H__
#define __EXAMPLE_PROGRAM_H__

using AttributeList = std::vector<std::string>;
using UniformList = std::vector<std::string>;

class Uniform
{
private:
    bool bad;

public:
    Uniform() : bad(true){}; /* Default initialized Uniform is invalid. Warn. */

    template <typename T>
    Uniform &operator<<(const T &other);
    Uniform &operator<<(const float &other);
    Uniform &operator=(const std::string &other);

    bool is_bad(); /* Internal usage. */
    std::string identifier;
    GLuint loc;
};

template <typename T>
Uniform &Uniform::operator<<(const T &other)
{
    if (this->is_bad())
        return *this;

    // Error message should happen on the Program::operator[] overload.
    if (this->identifier.empty())
        return *this;

    /* generic glUniform{1234}fv buffer upload routines */
    if constexpr (std::is_same_v<T, glm::vec2>)
        glUniform2fv(this->loc, 1, glm::value_ptr(other));
    else if constexpr (std::is_same_v<T, glm::vec3>)
        glUniform3fv(this->loc, 1, glm::value_ptr(other));
    else if constexpr (std::is_same_v<T, glm::vec4>)
        glUniform4fv(this->loc, 1, glm::value_ptr(other));
    else if constexpr (std::is_same_v<T, glm::mat2>)
        glUniformMatrix2fv(this->loc, 1, false, glm::value_ptr(other));
    else if constexpr (std::is_same_v<T, glm::mat3>)
        glUniformMatrix3fv(this->loc, 1, false, glm::value_ptr(other));
    else if constexpr (std::is_same_v<T, glm::mat4>)
        glUniformMatrix4fv(this->loc, 1, false, glm::value_ptr(other));
    else if constexpr (std::is_same_v<T, float>)
        glUniform1f(this->loc, other);
    else if constexpr (std::is_same_v<T, int>)
        glUniform1i(this->loc, other);
    else
        static_assert(false, "Unknown uniform data type");

    return *this;
}

class Program
{
    //Provides Program::uniforms[] syntax
    //E.g.: prog->uniforms["alpha"] << 0.2f;
    //E.g.: prog->uniforms["uMVPmat"] << proj * view * model;
    struct Uniforms
    {
        Program &prog;
        Uniform &operator[](const std::string &idx);
        Uniforms(Program &prog) : prog(prog){};
    };

    struct Attributes
    {
        Program &prog;
        GLuint operator[](const std::string &idx);
        Attributes(Program &prog) : prog(prog){};
    };

    static Uniform bad;
    std::map<std::string, GLuint> attribute_map;
    std::map<std::string, Uniform> uniform_map;
    std::map<std::string, bool> bad_uniform_warned;
public:
    Program(const std::string &vert, const std::string &frag, 
            const AttributeList &attrs, const UniformList &uniforms);

    Program::Uniforms uniforms;
    Program::Attributes attributes;
    GLuint program, vert_shader, frag_shader;
    GLuint diffuse;

    void Active();
};

#endif /* __EXAMPLE_PROGRAM_H__ */