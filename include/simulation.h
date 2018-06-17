#ifndef _DSPROJECT_SIMULATION_H_
#define _DSPROJECT_SIMULATION_H_

#include "singleton.h"
#include "motion_state.h"

#include <btBulletDynamicsCommon.h>
#include <memory>

class Simulation : public Singleton<Simulation> {
public:
    Simulation();

    void add_rigidbody(btRigidBody* body);
    void update(float dt);

private:
    static const int MAX_PROXIES = 1024;
    static const btVector3 WORLD_MIN;
    static const btVector3 WORLD_MAX;

    std::unique_ptr<btAxisSweep3> broadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> collision_config;
    std::unique_ptr<btCollisionDispatcher> collision_dispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

    std::unique_ptr<btDiscreteDynamicsWorld> dynamic_world;
};

#define SIMULATION Simulation::get_singleton()

#endif
