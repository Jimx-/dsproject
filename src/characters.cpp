#include "log_manager.h"
#include "characters.h"
#include "simulation.h"
#include "controllers.h"
#include "map.h"
#include "character_manager.h"
#include "config.h"

#include <cmath>
#include <queue>
#define DISTANCE(p1, p2) sqrt((p1.x - p2.x) * (p1.x-p2.x) + (p1.z - p2.z) * (p1.z - p2.z))

const float PI = acos(-1.0f);
const float PI2 = PI * 2;

void BaseCharacter::init_model()
{
    model = load_model();
}

void BaseCharacter::draw(Renderer& renderer)
{
	renderer.push_matrix();
	auto pos = get_position();
	auto rot = get_rotation();
	auto axis = glm::axis(rot);
	renderer.translate(pos[0], pos[1], pos[2]);
	renderer.rotate(glm::angle(rot), axis[0], axis[1], axis[2]);
	intrinsic_transform(renderer);
	model->draw(renderer);
	renderer.pop_matrix();
}

void BaseCharacter::set_animation(InternString name)
{
	if (model) {
		model->start_animation(name);
	}
}

void BaseCharacter::set_rotation(float angle, glm::vec3 axis)
{
	set_rotation(glm::angleAxis(angle, axis));
}

void BaseCharacter::set_rotation(glm::vec3 euler_angles)
{
	set_rotation(glm::quat(euler_angles));
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

void RigidCharacter::set_angular_velocity(glm::vec3 velo)
{
	btVector3 v(velo.x, velo.y, velo.z);
	rigid_body->setAngularVelocity(v);
}

glm::vec3 RigidCharacter::get_position() const
{
	auto pos = rigid_body->getWorldTransform().getOrigin();
	return glm::vec3(pos.x(), pos.y(), pos.z());
}

void RigidCharacter::set_position(glm::vec3 pos)
{
	auto rot = rigid_body->getWorldTransform().getRotation();
	rigid_body->setWorldTransform(btTransform(rot, btVector3(pos.x, pos.y, pos.z)));
}

glm::quat RigidCharacter::get_rotation() const
{
	auto rot = rigid_body->getWorldTransform().getRotation();
	return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

void RigidCharacter::set_rotation(glm::quat rot)
{
	auto pos = rigid_body->getWorldTransform().getOrigin();
	rigid_body->setWorldTransform(btTransform(btQuaternion(rot.x, rot.y, rot.z, rot.w), pos));
}

void RigidCharacter::apply_impulse(glm::vec3 force, glm::vec3 rel)
{
	rigid_body->applyImpulse(btVector3(force.x, force.y, force.z),
		btVector3(rel.x, rel.y, rel.z));
}

const float MainCharacter::MAX_HP = 100.0f;
MainCharacter::MainCharacter()
{
    init_rigidbody({0.0f, 0.0f, 0.0f});
	hp = MAX_HP;
}

btRigidBody* MainCharacter::setup_rigid_body(const btTransform& trans)
{
	camera.reset(new Camera(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
	motion_state.reset(new CameraMotionState (trans, camera.get()));

	btCollisionShape* camShape = new btCapsuleShape(0.5, 1.5);
	camShape->setMargin(0.1);
	btScalar mass = 1;
	btVector3 camInertia(0, 0, 0);
	camShape->calculateLocalInertia(mass, camInertia);
	btRigidBody::btRigidBodyConstructionInfo camRigidBodyCI(mass, motion_state.get(), camShape, camInertia);
	btRigidBody* camRigidBody = new btRigidBody(camRigidBodyCI);
	camRigidBody->setAngularFactor(btVector3(0, 1, 0));
	camRigidBody->setActivationState(DISABLE_DEACTIVATION);

	return camRigidBody;
}

void MainCharacter::deal_damage(float damage)
{
	if (damage < hp) {
		hp -= damage;
		return;
	}

	hp = 0.0f;
	Controller::switch_controller("end_game");
}

SkeletonCharacter::SkeletonCharacter(glm::vec3 pos)
{
	init_rigidbody(pos);
    init_model();
	set_animation("idle");
    nstate = IDLE;
}

btRigidBody* SkeletonCharacter::setup_rigid_body(const btTransform& trans)
{
	btDefaultMotionState* motion_state = new btDefaultMotionState(trans);
	btCollisionShape* skeShape = new btCapsuleShape(0.5, 1.5);
	btScalar mass = 10;
	btVector3 skeInertia(0, 0, 0);
	skeShape->calculateLocalInertia(mass, skeInertia);
	btRigidBody::btRigidBodyConstructionInfo skeRigidBodyCI(mass, motion_state, skeShape, skeInertia);
	btRigidBody* skeRigidBody = new btRigidBody(skeRigidBodyCI);
	skeRigidBody->setAngularFactor(btVector3(0, 0, 0));
	skeRigidBody->setActivationState(DISABLE_DEACTIVATION);

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

void SkeletonCharacter::update(float dt)
{
	const float ATTACK_DIST_THRESHOLD = 1.0f;
	const float TURN_THRESHOLD = glm::radians(10.f);

	glm::vec3 pos = get_position();
	glm::vec3 pos_c = CHARACTER_MANAGER.main_char().get_camera().get_position();
	glm::vec3 velo = pos_c - pos;

	float velo_dir = ::atan2(velo.x, -velo.z);
	btScalar xform[16];
	rigid_body->getWorldTransform().getOpenGLMatrix(xform);
	float cur_dir = ::atan2(xform[8], -xform[10]);

	float diff1 = abs(velo_dir - cur_dir);
    float diff2 = abs((velo_dir + PI2) - cur_dir);
	float diff3 = abs(velo_dir - (cur_dir + PI2));
#define MIN(x, y) ((x) < (y) ? (x) : (y))
	float min_diff = MIN(MIN(diff1, diff2), diff3);
    if (min_diff == diff2) {
		velo_dir += PI2;
	} else if (min_diff == diff3) {
		cur_dir += PI2;
	}

    LogManager::get_singleton().info("V%f, C%f", velo_dir, cur_dir);
	switch (nstate){
		case IDLE:
			if(DISTANCE(pos, pos_c) <= 8 && DISTANCE(pos,pos_c) > ATTACK_DIST_THRESHOLD){
				set_animation("walk");
				set_linear_velocity(velo);
				nstate = WALK;
			}
			if(DISTANCE(pos,pos_c) <= ATTACK_DIST_THRESHOLD){
				set_animation("attack");
				nstate = ATTACK;
				CHARACTER_MANAGER.main_char().deal_damage(10.f);
				t = 0;
			}
			break;
		case WALK:
			if(DISTANCE(pos, pos_c) <= ATTACK_DIST_THRESHOLD){
				set_animation("attack");
				nstate = ATTACK;
				CHARACTER_MANAGER.main_char().deal_damage(10.f);
				t = 0;
			}
			else if(DISTANCE(pos,pos_c) > 8){
				set_animation("idle");
				nstate = IDLE;
			}
			if (min_diff > TURN_THRESHOLD) {
				if (cur_dir < velo_dir)
					set_angular_velocity({0, -5 * min_diff, 0});
				else
					set_angular_velocity({0, 5 * min_diff, 0});
			}
			set_linear_velocity(velo);
            break;
		case ATTACK:
			t+=dt;
			set_linear_velocity({0, 0, 0});
			set_angular_velocity({0, 0, 0});
			if(t > 3 && DISTANCE(pos,pos_c)> ATTACK_DIST_THRESHOLD && DISTANCE(pos,pos_c) < 8){
				set_animation("walk");
				set_linear_velocity(velo);
				nstate = WALK;
			}
			else if(t <= 3){
				nstate = ATTACK;
			}
			else if(DISTANCE(pos,pos_c) >= 8 || t>3){
				set_animation("idle");
				nstate = IDLE;
			}

	}
}

TrapItem::TrapItem(glm::vec3 pos) : StaticCharacter(pos)
{
	init_model();
    triggered = false;
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

void TrapItem::update(float dt)
{
	if (triggered) return;

	glm::vec3 main_pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
	main_pos[1] = 0.0f;
	if (DISTANCE(main_pos, pos) < 1.0f) {
		CHARACTER_MANAGER.main_char().deal_damage(10.0f);
		triggered = true;
	}
}

ChestTrapItem::ChestTrapItem(glm::vec3 pos) : pos(pos)
{
	init_rigidbody(pos);
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
	renderer.translate(-1.3f, -0.5f, 0.0f);
	renderer.scale(0.04f, 0.04f, 0.04f);
}

btRigidBody* ChestTrapItem::setup_rigid_body(const btTransform& trans)
{
	btDefaultMotionState* motion_state = new btDefaultMotionState(trans);
	btCollisionShape* cheShape = new btBoxShape(btVector3(0.4, 0.5, 0.2));
	btScalar mass = 100;
	btVector3 cheInertia(0, 0, 0);
	cheShape->calculateLocalInertia(mass, cheInertia);
	btRigidBody::btRigidBodyConstructionInfo cheRigidBodyCI(mass, motion_state, cheShape, cheInertia);
	btRigidBody* cheRigidBody = new btRigidBody(cheRigidBodyCI);

	return cheRigidBody;
}

void ChestTrapItem::update(float dt)
{
	if (triggered) return;

    glm::vec3 main_pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
    main_pos[1] = 0.0f;

    if (DISTANCE(main_pos, pos) < 2.0f) {
        CHARACTER_MANAGER.main_char().add_score(100);
        triggered = true;
    }
}

TorchItem::TorchItem(glm::vec3 pos)
{
	init_rigidbody(pos);
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
	renderer.translate(0.0f, -0.5f, 0.0f);
	renderer.scale(0.18f, 0.18f, 0.18f);
}

btRigidBody* TorchItem::setup_rigid_body(const btTransform& trans)
{
	btDefaultMotionState* motion_state = new btDefaultMotionState(trans);
	btCollisionShape* torShape = new btCylinderShape(btVector3(0.2, 0.5, 0.2));
	btScalar mass = 3;
	btVector3 torInertia(0, 0, 0);
	torShape->calculateLocalInertia(mass, torInertia);
	btRigidBody::btRigidBodyConstructionInfo torRigidBodyCI(mass, motion_state, torShape, torInertia);
	btRigidBody* torRigidBody = new btRigidBody(torRigidBodyCI);

	return torRigidBody;
}

BarrelItem::BarrelItem(glm::vec3 pos)
{
	init_rigidbody(pos);
	init_model();
}

PModel BarrelItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/Barrel1.FBX"));
	return ourModel;
}

AnimationModel* BarrelItem::load_model() const
{
	return new AnimationModel(_prepare_model());
}

void BarrelItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(0.0f, -0.5f, 0.0f);
	renderer.rotate(glm::radians(-90.0f), 1.0f, 0.0f, 0.0f);
	renderer.scale(0.006f, 0.006f, 0.006f);
}

btRigidBody* BarrelItem::setup_rigid_body(const btTransform& trans)
{
	btDefaultMotionState* motion_state = new btDefaultMotionState(trans);
	btCollisionShape* barShape = new btCylinderShape(btVector3(0.4, 0.5, 0.4));
	btScalar mass = 5;
	btVector3 barInertia(0, 0, 0);
	barShape->calculateLocalInertia(mass, barInertia);
	btRigidBody::btRigidBodyConstructionInfo barRigidBodyCI(mass, motion_state, barShape, barInertia);
	btRigidBody* barRigidBody = new btRigidBody(barRigidBodyCI);

	return barRigidBody;
}

ChestKeyItem::ChestKeyItem(glm::vec3 pos) : pos(pos)
{
	init_rigidbody(pos);
	init_model();
}

PModel ChestKeyItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/MedievalChest_02_open.fbx"));
	return ourModel;
}

AnimationModel* ChestKeyItem::load_model() const
{
	return new AnimationModel(_prepare_model());
}

void ChestKeyItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(-1.3f, -0.5f, 0.0f);
	renderer.scale(0.04f, 0.04f, 0.04f);
}

btRigidBody* ChestKeyItem::setup_rigid_body(const btTransform& trans)
{
	btDefaultMotionState* motion_state = new btDefaultMotionState(trans);
	btCollisionShape* cheShape = new btBoxShape(btVector3(0.4, 0.5, 0.2));
	btScalar mass = 100;
	btVector3 cheInertia(0, 0, 0);
	cheShape->calculateLocalInertia(mass, cheInertia);
	btRigidBody::btRigidBodyConstructionInfo cheRigidBodyCI(mass, motion_state, cheShape, cheInertia);
	btRigidBody* cheRigidBody = new btRigidBody(cheRigidBodyCI);

	return cheRigidBody;
}

void ChestKeyItem::update(float dt)
{
	glm::vec3 main_pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
	main_pos[1] = 0.0f;

	if (DISTANCE(main_pos, pos) < 2.0f) {
        Controller::switch_controller("end_game");
	}
;}
