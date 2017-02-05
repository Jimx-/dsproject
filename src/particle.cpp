#include "particle.h"

Billboard::Billboard(glm::vec3 pos, float width, float height, const std::string& tex) : pos(pos), Renderable(false), width(width), height(height)
{
	glGenVertexArrays(1, &vao);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), &pos, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	texture = MaterialTexture::create_texture(tex);
}

void Billboard::draw(Renderer& renderer)
{
	renderer.uniform("uBillboardWidth", width);
	renderer.uniform("uBillboardHeight", height);
	glBindVertexArray(vao);
	glActiveTexture(0);
	texture->bind(GL_TEXTURE0);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
}

FlameParticle::FlameParticle(glm::vec3 pos) : Particle(pos, 0.5f, 0.5f, "FlameParticle1_I.jpg")
{
	textures.push_back(MaterialTexture::create_texture("FlameParticle1_I.jpg"));
	textures.push_back(MaterialTexture::create_texture("FlameParticle2_I.jpg"));
	textures.push_back(MaterialTexture::create_texture("FlameParticle3_I.jpg"));
	textures.push_back(MaterialTexture::create_texture("FlameParticle4_I.jpg"));
	age = 0.0f;
	tex_idx = 0;
}

void FlameParticle::update(float dt)
{
#define FRAME_TIME	0.04f

	age += dt;
	if (age >= FRAME_TIME) {
		tex_idx = (tex_idx + 1) % textures.size();
		age = 0;
		set_texture(textures[tex_idx]);
	}
}
