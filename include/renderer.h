//
// Created by jimx on 16-12-16.
//

#ifndef WEEABOO_RENDERER_H
#define WEEABOO_RENDERER_H

#include "shader_program.h"
#include "intern_string.h"
#include "singleton.h"
#include "light.h"
#include "camera.h"

#include <map>
#include <stack>
#include <vector>
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

class Renderable;
using PRenderable = std::shared_ptr<Renderable>;
class Overlay;
using POverlay = std::shared_ptr<Overlay>;

class Renderer : public Singleton<Renderer> {
public:
    Renderer();

    void set_viewport(int width, int height);
    void update_camera(const Camera& camera);

    static const InternString LIGHTING_SHADER;
    static const InternString BONE_ANIM_SHADER;
    static const InternString GEOMETRY_PASS_SHADER;
    static const InternString LIGHTING_PASS_SHADER;
    static const InternString SSAO_SHADER;
    static const InternString SSAO_BLUR_SHADER;
    static const InternString DEPTH_MAP_SHADER;
	static const InternString HDR_BLEND_SHADER;
	static const InternString GAUSSIAN_BLUR_SHADER;
	static const InternString BILLBOARD_SHADER;
    static const InternString TEXT_OVERLAY_SHADER;

    static const GLuint DIFFUSE_TEXTURE_TARGET = GL_TEXTURE0;
    static const GLuint NORMAL_MAP_TARGET = GL_TEXTURE1;

    static const int MAX_LIGHTS = 32;

    void use_shader(InternString name);
    void uniform(ShaderProgram::UniformID id, float v0, float v1, float v2 = 0.0f, float v3 = 0.0f);
    void uniform(ShaderProgram::UniformID id, int i0);
    void uniform(ShaderProgram::UniformID id, float f0);
    void uniform(ShaderProgram::UniformID id, GLsizei count, GLboolean transpose, const GLfloat* mat);

    void begin_frame();
    void end_frame();

    void push_matrix();
    void pop_matrix();

    template <typename T>
    void translate(T x, T y, T z) {
        model = glm::translate(model, glm::vec3(x, y, z));
        update_mvp();
    }

    template <typename T>
    void rotate(T angle, T x, T y, T z) {
        model = glm::rotate(model, angle, glm::vec3(x, y, z));
        update_mvp();
    }

    template <typename T>
    void scale(T x, T y, T z) {
        model = glm::scale(model, glm::vec3(x, y, z));
        update_mvp();
    }

    void add_light(const glm::vec3& position, const glm::vec3& color, float linear = 0.5f, float quadratic = 1.0f);

    void enqueue_renderable(PRenderable renderable);
    void enqueue_overlay(POverlay overlay);

private:
    static const float Z_NEAR;
    static const float Z_FAR;
    static const float FOV;

    static const int SHADOW_WIDTH = 1024;
    static const int SHADOW_HEIGHT = 1024;
    static const float SHADOW_NEAR;
    static const float SHADOW_FAR;

    std::map<InternString, PShaderProgram> shaders;
    PShaderProgram current_shader;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    std::stack<glm::mat4> xforms;
	glm::vec3 view_pos;

    std::vector<PRenderable> render_queue;
    std::vector<POverlay> overlay_queue;

    std::vector<Light> lights;
    int shadow_map_light_index;

    GLuint gbuffer;
    GLuint rbo_depth;
    GLuint g_position;
    GLuint g_normal;
    GLuint g_albedo_spec;

    GLuint quad_VAO;
    GLuint quad_VBO;

    GLuint ssao_fbo;
    GLuint ssao_blur_fbo;
    std::vector<glm::vec3> ssao_kernel;
    GLuint ssao_noise_texture;
    GLuint ssao_color_buffer;
    GLuint ssao_color_buffer_blur;

    GLuint depth_map_fbo;
    GLuint depth_cubemap;

	GLuint hdr_fbo;
	GLuint hdr_buffers[2];
	GLuint bloom_blur_fbo[2];
	GLuint bloom_blur_buffers[2];

    void setup_gbuffer();
    void update_mvp();

    void setup_quad();
    void render_quad();

    void render_lighting_pass();

    void setup_SSAO();
    void SSAO_pass();

    void setup_shadow_map();
    void shadow_map_pass();

	void setup_HDR();
	void post_process_pass();

    void overlay_pass();
};

#define RENDERER Renderer::get_singleton()

#endif //WEEABOO_RENDERER_H

