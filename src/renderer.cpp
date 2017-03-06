//
// Created by jimx on 16-12-16.
//

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include "renderer.h"
#include "renderable.h"
#include "config.h"
#include "log_manager.h"
#include "exception.h"
#include "random_utils.h"
#include "character_manager.h"
template <>
Renderer* Singleton<Renderer>::singleton = nullptr;

const float Renderer::FOV = glm::radians(45.0f);
const float Renderer::Z_NEAR = 0.1f;
const float Renderer::Z_FAR = 100.0f;
const float Renderer::SHADOW_NEAR = 1.0f;
const float Renderer::SHADOW_FAR = 25.0f;

const GLuint MINIMAP_SIZE = 8;

Renderer::Renderer()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
    //glEnable(GL_CULL_FACE);

    /* setup matrix stack */
    xforms.push(glm::mat4());

    /* setup shaders */
    PShaderProgram geometry_pass(new ShaderProgram("resources/shaders/bone_animation.vert", "resources/shaders/gbuffer.frag"));
    shaders[GEOMETRY_PASS_SHADER] = geometry_pass;
    use_shader(GEOMETRY_PASS_SHADER);
    geometry_pass->uniform(ShaderProgram::DIFFUSE_TEXTURE, 0);
    geometry_pass->uniform(ShaderProgram::NORMAL_MAP, 1);

    PShaderProgram lighting_pass(new ShaderProgram("resources/shaders/screen_quad.vert", "resources/shaders/lighting.frag"));
    shaders[LIGHTING_PASS_SHADER] = lighting_pass;
    use_shader(LIGHTING_PASS_SHADER);
    lighting_pass->uniform(ShaderProgram::GBUFFER_POSITION, 0);
    lighting_pass->uniform(ShaderProgram::GBUFFER_NORMAL, 1);
    lighting_pass->uniform(ShaderProgram::GBUFFER_ALBEDO_SPEC, 2);
    lighting_pass->uniform("uSSAOInput", 3);
    lighting_pass->uniform("uDepthMap", 4);
    lighting_pass->uniform("uFarPlane", SHADOW_FAR);

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

    PShaderProgram depth_map_shader(new ShaderProgram("resources/shaders/depth_map.vert", "resources/shaders/depth_map.frag", "resources/shaders/depth_map.geom"));
    shaders[DEPTH_MAP_SHADER] = depth_map_shader;

	PShaderProgram hdr_blend_shader(new ShaderProgram("resources/shaders/screen_quad.vert", "resources/shaders/hdr_blend.frag"));
	shaders[HDR_BLEND_SHADER] = hdr_blend_shader;
	use_shader(HDR_BLEND_SHADER);
	hdr_blend_shader->uniform("uHDRInput", 0);
	hdr_blend_shader->uniform("uBloomBlur", 1);

	PShaderProgram gaussian_blur_shader(new ShaderProgram("resources/shaders/screen_quad.vert", "resources/shaders/gaussian_blur.frag"));
	shaders[GAUSSIAN_BLUR_SHADER] = gaussian_blur_shader;
	use_shader(GAUSSIAN_BLUR_SHADER);
	gaussian_blur_shader->uniform("uInput", 0);

	PShaderProgram billboard_shader(new ShaderProgram("resources/shaders/billboard.vert", "resources/shaders/billboard.frag", "resources/shaders/billboard.geom"));
	shaders[BILLBOARD_SHADER] = billboard_shader;
	use_shader(BILLBOARD_SHADER);
	billboard_shader->uniform("uTexture", 0);

    PShaderProgram text_shader(new ShaderProgram("resources/shaders/text_overlay.vert", "resources/shaders/text_overlay.frag"));
    shaders[TEXT_OVERLAY_SHADER] = text_shader;
    use_shader(TEXT_OVERLAY_SHADER);
    text_shader->uniform("uText", 0);

    PShaderProgram minimap_shader(new ShaderProgram("resources/shaders/minimap.vert", "resources/shaders/minimap.frag"));
    shaders[MINIMAP_SHADER] = minimap_shader;
    use_shader(MINIMAP_SHADER);
    minimap_shader->uniform("uTexture", 0);

    setup_gbuffer();
    setup_quad();
    setup_SSAO();
	setup_HDR();
    setup_shadow_map();
    setup_minimap();
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

void Renderer::setup_shadow_map()
{
    glGenFramebuffers(1, &depth_map_fbo);
    // Create depth cubemap texture
    glGenTextures(1, &depth_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    for (GLuint i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // Attach cubemap as depth map FBO's color buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        THROW_EXCEPT(E_RENDER_ENGINE_ERROR, "Renderer::setup_shadow_map()", "cannot setup shadow map buffer");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setup_HDR()
{
	glGenFramebuffers(1, &hdr_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
	glGenTextures(2, hdr_buffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hdr_buffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB16F, g_screen_width, g_screen_height, 0, GL_RGB, GL_FLOAT, NULL
			);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, hdr_buffers[i], 0
			);
	}
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	GLuint rbo_depth;
    glGenRenderbuffers(1, &rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_screen_width, g_screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(2, bloom_blur_fbo);
	glGenTextures(2, bloom_blur_buffers);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo[i]);
		glBindTexture(GL_TEXTURE_2D, bloom_blur_buffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB16F, g_screen_width, g_screen_height, 0, GL_RGB, GL_FLOAT, NULL
			);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_blur_buffers[i], 0
			);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer::set_viewport(int width, int height)
{
    projection = glm::perspective(FOV, width / (float) height, Z_NEAR, Z_FAR);
    g_screen_height = height;
    g_screen_width = width;
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
    render_queue.clear();
    overlay_queue.clear();
}

void Renderer::end_frame()
{
    model = xforms.top();

    shadow_map_pass();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    use_shader(GEOMETRY_PASS_SHADER);
    update_mvp();

    for (int i = 0; i < render_queue.size(); i++) {
		if (!render_queue[i]->is_opaque()) continue;
        render_queue[i]->draw(*this);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SSAO_pass();
    render_lighting_pass();
	post_process_pass();
    overlay_pass();

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

void Renderer::enqueue_renderable(PRenderable renderable)
{
    render_queue.push_back(renderable);
}

void Renderer::enqueue_overlay(POverlay overlay)
{
    overlay_queue.push_back(overlay);
}

void Renderer::update_camera(const Camera& camera)
{
    view_pos = camera.get_position();
    int min_index = 0;
    float min_dist = glm::distance(view_pos, lights[0].position);
    for (int i = 1; i < lights.size(); i++) {
        float dist = glm::distance(view_pos, lights[i].position);
        if (dist < min_dist) {
            min_dist = dist;
            min_index = i;
        }
    }
    shadow_map_light_index = min_index;

    view = camera.get_view_matrix();
    update_mvp();
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
    use_shader(LIGHTING_PASS_SHADER);
	glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	update_mvp();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_albedo_spec);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssao_color_buffer_blur);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    glm::vec3 shadow_light_pos = lights[shadow_map_light_index].position;
    uniform("uShadowLightPos", shadow_light_pos.x, shadow_light_pos.y, shadow_light_pos.z);
    uniform("uShadowLightIndex", shadow_map_light_index);

    GLuint lighting_program = shaders[LIGHTING_PASS_SHADER]->get_program();
    for (GLuint i = 0; i < lights.size(); i++)
    {
		glUniform3fv(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].position").c_str()), 1, &lights[i].position[0]);
        if (0) {
            glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
            glUniform3fv(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].color").c_str()),
                         1, &red[0]);
        } else {
            glUniform3fv(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].color").c_str()),
                         1, &lights[i].color[0]);
        }
        // Update attenuation parameters and calculate radius
        glUniform1f(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].linear").c_str()), lights[i].linear);
        glUniform1f(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].quadratic").c_str()), lights[i].quadratic);

        float intensity = RandomUtils::random_int(900, 1000) / 1000.0;
        glUniform1f(glGetUniformLocation(lighting_program, ("uLights[" + std::to_string(i) + "].intensity").c_str()), intensity);
    }
    uniform(ShaderProgram::VIEW_POS, view_pos.x, view_pos.y, view_pos.z);

    render_quad();

	/* blit depth buffer */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdr_fbo); 
	glBlitFramebuffer(0, 0, g_screen_width, g_screen_height, 0, 0, g_screen_width, g_screen_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	/* forward shading pass */
    use_shader(BILLBOARD_SHADER);
	glm::mat4 vp = projection * view;
	uniform(ShaderProgram::VP, 1, false, glm::value_ptr(vp));
	uniform(ShaderProgram::VIEW, 1, false, glm::value_ptr(view));

    for (int i = 0; i < render_queue.size(); i++) {
		if (render_queue[i]->is_opaque()) continue;
        render_queue[i]->draw(*this);
    }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void Renderer::shadow_map_pass()
{
    glm::vec3 lightPos = lights[shadow_map_light_index].position;

    float aspect = (float) SHADOW_WIDTH / (float) SHADOW_HEIGHT;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, SHADOW_NEAR, SHADOW_FAR);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0,  0.0,  0.0), glm::vec3(0.0, -1.0,  0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  1.0,  0.0), glm::vec3(0.0,  0.0,  1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, -1.0,  0.0), glm::vec3(0.0,  0.0, -1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0,  1.0), glm::vec3(0.0, -1.0,  0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,  0.0, -1.0), glm::vec3(0.0, -1.0,  0.0)));

    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    use_shader(DEPTH_MAP_SHADER);
    update_mvp();
    glClear(GL_DEPTH_BUFFER_BIT);
    for (GLuint i = 0; i < 6; ++i)
        glUniformMatrix4fv(glGetUniformLocation(shaders[DEPTH_MAP_SHADER]->get_program(), ("uShadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
    uniform("uFarPlane", SHADOW_FAR);
    uniform("uLightPos", lightPos[0], lightPos[1], lightPos[2]);

    for (int i = 0; i < render_queue.size(); i++) {
		if (!render_queue[i]->is_opaque()) continue;
        render_queue[i]->draw(*this);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, g_screen_width, g_screen_height);
}

void Renderer::post_process_pass()
{
	use_shader(GAUSSIAN_BLUR_SHADER);
	GLboolean horizontal = true, first_iteration = true;
	GLuint amount = 10;
	for (GLuint i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo[horizontal]);
		uniform("uHorizontal", (int)horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(
			GL_TEXTURE_2D, first_iteration ? hdr_buffers[1] : bloom_blur_buffers[!horizontal]
			);
		render_quad();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	use_shader(HDR_BLEND_SHADER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdr_buffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloom_blur_buffers[horizontal]);

	render_quad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setup_minimap()
{
    glGenVertexArrays(1, &minimap_VAO);
    glGenBuffers(1, &minimap_VBO);
    glBindVertexArray(minimap_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, minimap_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::draw_minimap()
{
    GLfloat minimap_verts[24];

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(minimap_VAO);

    auto pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
    int si = (int)(pos[0] / Map::TILE_SIZE), sj = (int)(pos[2] / Map::TILE_SIZE);

    float radius = 0.15f;
    float x_span = radius * 2 * g_screen_height / MINIMAP_SIZE;
    float y_span = radius * 2 * g_screen_height / MINIMAP_SIZE;
    uniform("uRadius", x_span * MINIMAP_SIZE / 2);

    float centre_x = g_screen_width - 30 - x_span * MINIMAP_SIZE / 2;
    float centre_y = g_screen_height - 30 - y_span * MINIMAP_SIZE / 2;
    uniform("uCentre", centre_x, centre_y);

    GLfloat yaw = CHARACTER_MANAGER.main_char().get_camera().get_yaw();
    glm::mat4 proj = glm::ortho(0.0f, (float)g_screen_width, 0.0f, (float)g_screen_height);
    glm::mat4 model =
        glm::translate(glm::mat4(), {centre_x, centre_y, 0.0f}) *
        glm::rotate(glm::mat4(), glm::radians(yaw + 90.0f), glm::vec3(0.f, 0.f, 1.f));
    uniform("uModel", 1, false, glm::value_ptr(model));
    uniform("uProjection", 1, false, glm::value_ptr(proj));

    PMaterialTexture texture = MaterialTexture::create_texture("dungeon.png");
    texture->bind(Renderer::DIFFUSE_TEXTURE_TARGET);

    int hv = MINIMAP_SIZE / 2;
    for (int i = -hv; i < hv; i++) {
        for (int j = -hv; j < hv; j++) {
            int index = 0;

#define ADD_VERTEX(pos, texcoord) \
    minimap_verts[index++] = pos[0]; \
    minimap_verts[index++] = pos[1]; \
    minimap_verts[index++] = texcoord[0]; \
    minimap_verts[index++] = texcoord[1]; \

            std::vector<glm::vec2> texcs{{2.0, 0}, {2.0, 0}, {2.0, 0}, {2.0, 0}};
            int map_i = i + si, map_j = sj - j;
            if (!(map_i < 0 || map_i >= g_map_width || map_j < 0 || map_j >= g_map_height)) {
                char tile = g_map->get_tile(map_i, map_j);
                if (tile == MapGenerator::Tile::Floor || tile == MapGenerator::Spawn || tile == MapGenerator::Traps ||
                    tile == MapGenerator::Torch || tile == MapGenerator::Treasure_traps || tile == MapGenerator::ClosedDoor ||
                    tile == MapGenerator::OpenDoor || tile == MapGenerator::Player || tile == MapGenerator::Corridor ||
                    tile == MapGenerator::Key) {
                    texcs[0] = {0.0f, 0.0f};
                    texcs[1] = {0.5f, 0.0f};
                    texcs[2] = {0.0f, 0.5f};
                    texcs[3] = {0.5f, 0.5f};
                } else if (tile == MapGenerator::Tile::Wall) {
                    texcs[0] = {0.5f, 0.0f};
                    texcs[1] = {1.0f, 0.0f};
                    texcs[2] = {0.5f, 0.5f};
                    texcs[3] = {1.0f, 0.5f};
                }
            }


#define GEN_POS(i, j)  (i) * x_span, (j) * y_span
            glm::vec2 vertices[4] = {
                { GEN_POS(i, j) },
                { GEN_POS(i + 1, j) },
                { GEN_POS(i, j + 1) },
                { GEN_POS(i + 1, j + 1) },
            };

            ADD_VERTEX(vertices[0], texcs[0]);
            ADD_VERTEX(vertices[1], texcs[1]);
            ADD_VERTEX(vertices[2], texcs[2]);

            ADD_VERTEX(vertices[1], texcs[1]);
            ADD_VERTEX(vertices[3], texcs[3]);
            ADD_VERTEX(vertices[2], texcs[2]);

            glBindBuffer(GL_ARRAY_BUFFER, minimap_VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(minimap_verts), minimap_verts);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::overlay_pass()
{
    use_shader(TEXT_OVERLAY_SHADER);
    glm::mat4 proj = glm::ortho(0.0f, (float)g_screen_width, 0.0f, (float)g_screen_height);
    uniform("uProjection", 1, false, glm::value_ptr(proj));

    for (int i = 0; i < overlay_queue.size(); i++) {
        if (overlay_queue[i]->get_technique() != Overlay::Technique::TEXT) continue;
        overlay_queue[i]->draw(*this);
    }

    use_shader(MINIMAP_SHADER);
    draw_minimap();
}

