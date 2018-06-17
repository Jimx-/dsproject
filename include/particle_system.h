#ifndef DSPROJECT_PARTICLE_SYSTEM_H
#define DSPROJECT_PARTICLE_SYSTEM_H

#include "singleton.h"
#include "particle.h"

#include <type_traits>

class ParticleSystem : public Singleton<ParticleSystem> {
public:
    template <typename T, typename ... Args>
    typename std::enable_if<std::is_base_of<Particle, typename std::decay<T>::type>::value>::type spawn_particle(Args&&... args)
    {
        particles.push_back(std::shared_ptr<T>(new T(std::forward<Args>(args)...)));
    }

	void update(float dt);
	void submit(Renderer& renderer);

private:
	std::vector<std::shared_ptr<Particle> > particles;
};

#define PARTICLE_SYSTEM	ParticleSystem::get_singleton()

#endif
