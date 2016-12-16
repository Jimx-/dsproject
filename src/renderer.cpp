//
// Created by jimx on 16-12-16.
//

#include "renderer.h"
#include "log_manager.h"

template <>
Renderer* Singleton<Renderer>::singleton = nullptr;

Renderer::Renderer()
{
    /* setup shaders */
    PShaderProgram lighting(new ShaderProgram("resources/shaders/lighting.vert", "resources/shaders/lighting.frag"));
    shaders[LIGHTING_SHADER] = lighting;
    use_shader(LIGHTING_SHADER);
}

void Renderer::use_shader(InternString name)
{
    auto it = shaders.find(name);
    if (it == shaders.end()) {
        LOG.error("RENDERER::no such shader '%s'", name.c_str());
        return;
    }

    current_shader = it->second;
    current_shader->bind();
}

void Renderer::uniform(ShaderProgram::UniformID id, float v0, float v1, float v2, float v3)
{
    if (!current_shader) return;
    current_shader->uniform(id, v0, v1, v2, v3);
}

void Renderer::uniform(ShaderProgram::UniformID id, int i0)
{
    if (!current_shader) return;
    current_shader->uniform(id, i0);
}

void Renderer::uniform(ShaderProgram::UniformID id, GLsizei count, GLboolean transpose, const GLfloat* mat)
{
    if (!current_shader) return;
    current_shader->uniform(id, count, transpose, mat);
}


