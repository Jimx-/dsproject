//
// Created by jimx on 16-12-16.
//

#include <glm/gtc/matrix_transform.hpp>
#include "renderer.h"
#include "log_manager.h"

template <>
Renderer* Singleton<Renderer>::singleton = nullptr;

const float Renderer::FOV = 45.0f;
const float Renderer::Z_NEAR = 0.1f;
const float Renderer::Z_FAR = 100.0f;

Renderer::Renderer()
{
    glClearColor(0.5f, 0.5f, 0.3f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);

    /* setup shaders */
    PShaderProgram lighting(new ShaderProgram("resources/shaders/lighting.vert", "resources/shaders/lighting.frag"));
    shaders[LIGHTING_SHADER] = lighting;
    lighting->uniform(ShaderProgram::DIFFUSE_TEXTURE, 0);

    PShaderProgram bone_animation(new ShaderProgram("resources/shaders/bone_animation.vert", "resources/shaders/lighting.frag"));
    shaders[BONE_ANIM_SHADER] = bone_animation;
    bone_animation->uniform(ShaderProgram::DIFFUSE_TEXTURE, 0);
}


void Renderer::set_viewport(int width, int height)
{
    projection = glm::perspective(FOV, width / (float) height, Z_NEAR, Z_FAR);
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


