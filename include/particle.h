#ifndef DSPROJECT_PARTICLE_H
#define DSPROJECT_PARTICLE_H

#include "renderable.h"
#include "material.h"

#include <GLFW/glfw3.h>
#include <string>

class Billboard : public Renderable {
public:
	Billboard(glm::vec3 pos, float width = 1.0f, float height = 1.0f, const std::string& tex = "");
	virtual void draw(Renderer& renderer) override;

	void set_texture(PMaterialTexture tex) { texture = tex; }
private:
	glm::vec3 pos;
	GLuint vao;
	float width, height;

	PMaterialTexture texture;
};

class Particle : public Billboard {
public:
	Particle(glm::vec3 pos, float width = 1.0f, float height = 1.0f, const std::string& tex = "") : Billboard(pos, width, height, tex) { }

	virtual void update(float dt) { }
};

class FlameParticle : public Particle {
public:
	FlameParticle(glm::vec3 pos);

	virtual void update(float dt) override;

private:
	std::vector<PMaterialTexture> textures;
	float age;
	int tex_idx;
};

#endif
