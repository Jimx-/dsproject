//
// Created by jimx on 16-12-16.
//

#ifndef WEEABOO_RENDERER_H
#define WEEABOO_RENDERER_H

#include "shader_program.h"
#include "intern_string.h"
#include "singleton.h"

#include <map>
#include <string>
#include <glad/glad.h>

class Renderer : public Singleton<Renderer> {
public:
    Renderer();

    void set_viewport(int width, int height);

    static const InternString LIGHTING_SHADER;
    static const InternString BONE_ANIM_SHADER;

    static const GLuint DIFFUSE_TEXTURE_TARGET = GL_TEXTURE0;

    void use_shader(InternString name);
    void uniform(ShaderProgram::UniformID id, float v0, float v1, float v2 = 0.0f, float v3 = 0.0f);
    void uniform(ShaderProgram::UniformID id, int i0);
    void uniform(ShaderProgram::UniformID id, GLsizei count, GLboolean transpose, const GLfloat* mat);

private:
    std::map<InternString, PShaderProgram> shaders;
    PShaderProgram current_shader;

    glm::mat4 projection;

    static const float Z_NEAR;
    static const float Z_FAR;
    static const float FOV;
};

#define RENDERER Renderer::get_singleton()

#endif //WEEABOO_RENDERER_H

