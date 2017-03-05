#include "log_manager.h"
#include "characters.h"
#include "simulation.h"
#include "map.h"

void BaseCharacter::init_model()
{
    model = load_model();
}

void BaseCharacter::draw(Renderer& renderer)
{
	renderer.push_matrix();
	auto pos = get_position();
	renderer.translate(pos[0], pos[1], pos[2]);
	intrinsic_transform(renderer);
	model->draw(renderer);
	renderer.pop_matrix();
}

StaticCharacter::StaticCharacter(glm::vec3 pos) : pos(pos)
{

}

void RigidCharacter::init_rigidbody(glm::vec3 pos)
{
	btVector3 pv(pos.x, pos.y, pos.z);
	btQuaternion q(0.0f, 0.0f, 0.0f, 1.0f);
	rigid_body.reset(setup_rigid_body(btTransform(q, pv)));
	SIMULATION.add_rigidbody(rigid_body.get());
}

void RigidCharacter::set_linear_velocity(glm::vec3 velo)
{
	btVector3 v(velo.x, velo.y, velo.z);
	rigid_body->setLinearVelocity(v);
}

glm::vec3 RigidCharacter::get_position() const
{
	auto pos = rigid_body->getWorldTransform().getOrigin();
	return glm::vec3(pos.x(), pos.y(), pos.z());
}

void RigidCharacter::set_position(glm::vec3 pos)
{
	rigid_body->setWorldTransform(btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(pos.x, pos.y, pos.z)));
}

void RigidCharacter::apply_impulse(glm::vec3 force, glm::vec3 rel)
{
	rigid_body->applyImpulse(btVector3(force.x, force.y, force.z),
		btVector3(rel.x, rel.y, rel.z));
}

MainCharacter::MainCharacter()
{
    init_rigidbody({0.0f, 0.0f, 0.0f});
}

btRigidBody* MainCharacter::setup_rigid_body(const btTransform& trans)
{
	camera.reset(new Camera(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
	motion_state.reset(new CameraMotionState (trans, camera.get()));

	btCollisionShape* camShape = new btCapsuleShape(0.5, 1.5);
	btScalar mass = 1;
	btVector3 camInertia(0, 0, 0);
	camShape->calculateLocalInertia(mass, camInertia);
	btRigidBody::btRigidBodyConstructionInfo camRigidBodyCI(mass, motion_state.get(), camShape, camInertia);
	btRigidBody* camRigidBody = new btRigidBody(camRigidBodyCI);
	camRigidBody->setAngularFactor(btVector3(0, 1, 0));

	return camRigidBody;
}

SkeletonCharacter::SkeletonCharacter(glm::vec3 pos)
{
	init_rigidbody(pos);
    init_model();
    model->start_animation("idle");
}

btRigidBody* SkeletonCharacter::setup_rigid_body(const btTransform& trans)
{
	btDefaultMotionState* motion_state = new btDefaultMotionState(trans);
	btCollisionShape* skeShape = new btCapsuleShape(0.5, 1.5);
	btScalar mass = 1;
	btVector3 skeInertia(0, 0, 0);
	skeShape->calculateLocalInertia(mass, skeInertia);
	btRigidBody::btRigidBodyConstructionInfo skeRigidBodyCI(mass, motion_state, skeShape, skeInertia);
	btRigidBody* skeRigidBody = new btRigidBody(skeRigidBodyCI);
	skeRigidBody->setAngularFactor(btVector3(0, 1, 0));

	return skeRigidBody;
}

PModel SkeletonCharacter::_prepare_model()
{
    static PModel ourModel(new Model("resources/models/skeleton.FBX"));
	static bool first = true;
	if (first) {
		ourModel->load_animation("walk", "resources/animations/skeleton_onehand_walk.FBX");
		ourModel->load_animation("attack", "resources/animations/skeleton_onehand_attack.FBX");
		ourModel->load_animation("idle", "resources/animations/skeleton_onehand_idle.FBX");
		first = false;
	}
    return ourModel;
}

AnimationModel* SkeletonCharacter::load_model() const
{
    return new AnimationModel(_prepare_model());
}

void SkeletonCharacter::intrinsic_transform(Renderer& renderer) 
{
	renderer.translate(0.0f, -1.25f, 0.0f);
	renderer.scale(0.02f, 0.02f, 0.02f);
}

TrapItem::TrapItem(glm::vec3 pos) : StaticCharacter(pos)
{
	init_model();
	//model->start_animation("trigger");
}

PModel TrapItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/Spike-trap.fbx"));
	static bool first = true;
	if (first) {
		ourModel->load_animation("trigger", "resources/models/Spike-trap.fbx", 1);
		first = false;
	}
	return ourModel;
}

AnimationModel* TrapItem::load_model() const
{
	return new AnimationModel(_prepare_model());
}

void TrapItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(0.5f * Map::TILE_SIZE, 0.0f, 0.5f * Map::TILE_SIZE);
	renderer.rotate(glm::radians(-90.0f), 1.0f, 0.0f, 0.0f);
	renderer.scale(0.45f, 0.45f, 0.45f);
}

ChestTrapItem::ChestTrapItem(glm::vec3 pos) : StaticCharacter(pos)
{
	init_model();
}

PModel ChestTrapItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/MedievalChest_01_open.fbx"));
	return ourModel;
}

AnimationModel* ChestTrapItem::load_model() const
{
	return new AnimationModel(_prepare_model());
}

void ChestTrapItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(-0.5f * Map::TILE_SIZE, 0.0f, 0.5f * Map::TILE_SIZE);
	renderer.scale(0.04f, 0.04f, 0.04f);
}

TorchItem::TorchItem(glm::vec3 pos) : StaticCharacter(pos)
{
	init_model();
}

PModel TorchItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/torch4.obj"));
	return ourModel;
}

AnimationModel* TorchItem::load_model() const
{
	return new AnimationModel(_prepare_model());
}

void TorchItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(0.5f * Map::TILE_SIZE, 0.0f, 0.5f * Map::TILE_SIZE);
	renderer.scale(0.18f, 0.18f, 0.18f);
}
