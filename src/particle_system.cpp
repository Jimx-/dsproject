#include "particle_system.h"

template<>
ParticleSystem* Singleton<ParticleSystem>::singleton = nullptr;

void ParticleSystem::update(float dt)
{
	for (int i = 0; i < particles.size(); i++) {
		particles[i]->update(dt);
	}
}

void ParticleSystem::submit(Renderer& renderer)
{
	for (auto& p : particles) {
		renderer.enqueue_renderable(p);
	}
}
