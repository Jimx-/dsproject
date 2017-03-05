#include "simulation.h"

template<>
Simulation* Singleton<Simulation>::singleton = nullptr;

const btVector3 Simulation::WORLD_MIN(-1000, -1000, -100);
const btVector3 Simulation::WORLD_MAX(1000, 1000, 100);

Simulation::Simulation()
{
    broadphase.reset(new btAxisSweep3(WORLD_MIN, WORLD_MAX, MAX_PROXIES));
    collision_config.reset(new btDefaultCollisionConfiguration());
    collision_dispatcher.reset(new btCollisionDispatcher(collision_config.get()));
    solver.reset(new btSequentialImpulseConstraintSolver);

    dynamic_world.reset(new btDiscreteDynamicsWorld(collision_dispatcher.get(), broadphase.get(), solver.get(), collision_config.get()));
    dynamic_world->setGravity({0.0f, -10.0f, 0.0f});

    /* ground */
    btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
    btRigidBody::btRigidBodyConstructionInfo
        groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamic_world->addRigidBody(groundRigidBody);
}

void Simulation::add_rigidbody(btRigidBody* body)
{
    dynamic_world->addRigidBody(body);
}

void Simulation::update(float dt)
{
    dynamic_world->stepSimulation(dt, 10);
}
