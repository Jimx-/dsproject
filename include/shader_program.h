//
// Created by dollars on 16-12-13.
//

#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "intern_string.h"
#include <glad/glad.h>
#include <memory>
#include <map>

class ShaderProgram {
public:
    typedef InternString UniformID;

    static const InternString MVP;
    static const InternString VP;
    static const InternString MODEL;
    static const InternString VIEW;
    static const InternString VIEW_POS;
    static const InternString PROJECTION;
    static const InternString DIFFUSE_TEXTURE;
    static const InternString NORMAL_MAP;
    static const InternString BONE_TRANSFORMS;
    static const InternString MAT_ROUGHNESS;
    static const InternString MAT_METALLIC;

    static const InternString GBUFFER_POSITION;
    static const InternString GBUFFER_NORMAL;
    static const InternString GBUFFER_ALBEDO_SPEC;

    static const int MAX_BONE_TRANSFORMS = 100;

    struct Binding {
        int first, second;

        Binding(int first = -1, int second = -1) : first(first), second(second) { }
    };

    ShaderProgram(const char * vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    GLuint get_program() const { return program; }

    void bind();
    void unbind();

    void uniform(UniformID id, float v0, float v1, float v2 = 0.0f, float v3 = 0.0f);
    void uniform(UniformID id, int i0);
    void uniform(UniformID id, float f0);
    void uniform(UniformID id, GLsizei count, GLboolean transpose, const GLfloat* mat);

private:
    GLuint program;
    std::map<UniformID, std::pair<int, GLenum> > uniforms;

    Binding get_uniform_binding(UniformID id);
    void uniform(const Binding& b, float v0, float v1, float v2, float v3);
    void uniform(const Binding& b, int i0);
    void uniform(const Binding& b, float f0);
    void uniform(const Binding& b, GLsizei count, GLboolean transpose, const GLfloat* mat);
};

using PShaderProgram = std::shared_ptr<ShaderProgram>;

#endif // SHADERPROGRAM_H

