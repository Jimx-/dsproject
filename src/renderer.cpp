//
// Created by jimx on 16-12-16.
//

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include "renderer.h"
#include "config.h"
#include "log_manager.h"
#include "exception.h"

template <>
Renderer* Singleton<Renderer>::singleton = nullptr;

const float Renderer::FOV = 45.0f;
const float Renderer::Z_NEAR = 0.1f;
const float Renderer::Z_FAR = 100.0f;

static glm::vec3 view_pos(0.0f, 0.0f, 4.0f);

Renderer::Renderer()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);

    /* setup matrix stack */
    xforms.push(glm::mat4());

    /* setup shaders */
    PShaderProgram geometry_pass(new ShaderProgram("resources/shaders/bone_animation.vert", "resources/shaders/gbuffer.frag"));
    shaders[GEOMETRY_PASS_SHADER] = geometry_pass;
    use_shader(GEOMETRY_PASS_SHADER);
    geometry_pass->uniform(ShaderProgram::DIFFUSE_TEXTURE, 0);

    PShaderProgram lighting_pass(new ShaderProgram("resources/shaders/screen_quad.vert", "resources/shaders/lighting.frag"));
    shaders[LIGHTING_PASS_SHADER] = lighting_pass;
    use_shader(LIGHTING_PASS_SHADER);
    lighting_pass->uniform(ShaderProgram::GBUFFER_POSITION, 0);
    lighting_pass->uniform(ShaderProgram::GBUFFER_NORMAL, 1);
    lighting_pass->uniform(ShaderProgram::GBUFFER_ALBEDO_SPEC, 2);
    lighting_pass->uniform("uSSAOInput", 3);

    PShaderProgram ssao_shader(new ShaderProgram("resources/shaders/screen_quad.vert", "resources/shaders/ssao.frag"));
    shaders[SSAO_SHADER] = ssao_shader;
    use_shader(SSAO_SHADER);
    ssao_shader->uniform(ShaderProgram::GBUFFER_POSITION, 0);
    ssao_shader->uniform(ShaderProgram::GBUFFER_NORMAL, 1);
    ssao_shader->uniform("uNoiseTexture", 2);

    PShaderProgram ssao_blur_shader(new ShaderProgram("resources/shaders/screen_quad.vert", "resources/shaders/ssao_blur.frag"));
    shaders[SSAO_BLUR_SHADER] = ssao_blur_shader;
    use_shader(SSAO_BLUR_SHADER);
    ssao_blur_shader->uniform("uSSAOInput", 0);

    setup_gbuffer();
    setup_quad();
    setup_SSAO();
}

void Renderer::setup_gbuffer()
{
    glGenFramebuffers(1, &gbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
    // - Position color buffer
    glGenTextures(1, &g_position);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_screen_width, g_screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);
    // - Normal color buffer
    glGenTextures(1, &g_normal);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, g_screen_width, g_screen_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);
    // - Color + Specular color buffer
    glGenTextures(1, &g_albedo_spec);
    glBindTexture(GL_TEXTURE_2D, g_albedo_spec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_screen_width, g_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_albedo_spec, 0);
    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_screen_width, g_screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

    // - Finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        THROW_EXCEPT(E_RENDER_ENGINE_ERROR, "Renderer::setup_gbuffer()", "cannot setup geometry buffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setup_SSAO()
{
    // Sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for (GLuint i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = GLfloat(i) / 64.0;
#define lerp(a, b, t) (t * b + (1 - (t)) * a)
        // Scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssao_kernel.push_back(sample);
    }

    // Noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &ssao_noise_texture);
    glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenFramebuffers(1, &ssao_fbo);
    glGenFramebuffers(1, &ssao_blur_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    // - SSAO color buffer
    glGenTextures(1, &ssao_color_buffer);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_screen_width, g_screen_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_buffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        THROW_EXCEPT(E_RENDER_ENGINE_ERROR, "Renderer::setup_SSAO()", "cannot setup SSAO buffer");
    // - and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
    glGenTextures(1, &ssao_color_buffer_blur);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buffer_blur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g_screen_width, g_screen_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssao_color_buffer_blur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        THROW_EXCEPT(E_RENDER_ENGINE_ERROR, "Renderer::setup_SSAO()", "cannot setup SSAO blur buffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void Renderer::uniform(ShaderProgram::UniformID id, float f0)
{
    if (!current_shader) return;
    current_shader->uniform(id, f0);
}

void Renderer::uniform(ShaderProgram::UniformID id, GLsizei count, GLboolean transpose, const GLfloat* mat)
{
    if (!current_shader) return;
    current_shader->uniform(id, count, transpose, mat);
}

void Renderer::update_mvp()
{
    glm::mat4 mvp = projection * view * model;

    for (auto it : shaders) {
        it.second->uniform(ShaderProgram::MVP, 1, false, glm::value_ptr(mvp));
        it.second->uniform(ShaderProgram::MODEL, 1, false, glm::value_ptr(model));
    }
}

void Renderer::begin_frame()
{
    model = xforms.top();
    view = glm::lookAt(view_pos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    update_mvp();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    use_shader(GEOMETRY_PASS_SHADER);
}

void Renderer::end_frame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SSAO_pass();
    render_lighting_pass();

    while (xforms.size() > 1) xforms.pop();
}

void Renderer::push_matrix()
{
    xforms.push(model);
}

void Renderer::pop_matrix()
{
    model = xforms.top();
    xforms.pop();
    update_mvp();
}

void Renderer::add_light(const glm::vec3& position, const glm::vec3& color, float linear, float quadratic)
{
    if (lights.size() >= MAX_LIGHTS) return;
    lights.push_back(Light(position, color, linear, quadratic));
}

void Renderer::setup_quad()
{
    GLfloat quadVertices[] = {
        // Positions        // Texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    // Setup plane VAO
    glGenVertexArrays(1, &quad_VAO);
    glGenBuffers(1, &quad_VBO);
    glBindVertexArray(quad_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

void Renderer::render_quad()
{
    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::render_lighting_pass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    use_shader(LIGHTING_PASS_SHADER);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_albedo_spec);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buffer_blur);

    GLuint lighting_program = shaders[LIGHTING_PASS_SHADER]->get_program();
    for (GLuint i = 0; i < lights.size(); i++)
    {
        glUniform3fv(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].position").c_str()), 1, &lights[i].position[0]);
        glUniform3fv(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].color").c_str()), 1, &lights[i].color[0]);
        // Update attenuation parameters and calculate radius
        glUniform1f(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].linear").c_str()), lights[i].linear);
        glUniform1f(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].quadratic").c_str()), lights[i].quadratic);
    }
    uniform(ShaderProgram::VIEW_POS, view_pos.x, view_pos.y, view_pos.z);

    render_quad();
}

void Renderer::SSAO_pass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    use_shader(SSAO_SHADER);

    glBindFramebuffer(GL_FRAMEBUFFER, ssao_fbo);
    uniform(ShaderProgram::PROJECTION, 1, false, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ssao_noise_texture);

    for (GLuint i = 0; i < 64; ++i)
        glUniform3fv(glGetUniformLocation(shaders[SSAO_SHADER]->get_program(), ("uSamples[" + std::to_string(i) + "]").c_str()), 1, &ssao_kernel[i][0]);

    render_quad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, ssao_blur_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    use_shader(SSAO_BLUR_SHADER);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buffer);
    render_quad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
