#ifndef WEEABOO_CHARACTERS_H
#define WEEABOO_CHARACTERS_H

#include "renderable.h"
#include "motion_state.h"
#include "animation_model.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class BaseCharacter : public Renderable {
public:
	virtual glm::vec3 get_position() const = 0;
	virtual void set_position(glm::vec3 pos) = 0;
	virtual glm::quat get_rotation() const = 0;
	virtual void set_rotation(glm::quat rot) = 0;
	virtual void set_rotation(float angle, glm::vec3 axis);
	virtual void set_rotation(glm::vec3 euler_angles);

	virtual void set_linear_velocity(glm::vec3 velo) = 0;
	void set_animation(InternString name);
	virtual void apply_impulse(glm::vec3 force, glm::vec3 rel = { 0.0f, 0.0f, 0.0f }) = 0;

	virtual void draw(Renderer& renderer) override;
	virtual void update(float dt) { }

protected:
    void init_model();
    virtual AnimationModel* load_model() const = 0;
	virtual void intrinsic_transform(Renderer&) { }

    AnimationModel* model;
};

using PCharacter = std::shared_ptr<BaseCharacter>;

class StaticCharacter : public BaseCharacter {
public:
	virtual glm::vec3 get_position() const { return pos; }
	virtual void set_position(glm::vec3 pos) { this->pos = pos; }
	virtual glm::quat get_rotation() const { return rot; }
	virtual void set_rotation(glm::quat rot) { this->rot = rot; }

	virtual void set_linear_velocity(glm::vec3 velo) {}
	virtual void apply_impulse(glm::vec3 force, glm::vec3 rel = { 0.0f, 0.0f, 0.0f }) {}

protected:
	StaticCharacter(glm::vec3 pos);

	glm::vec3 pos;
	glm::quat rot;
};

class RigidCharacter : public BaseCharacter {
public:
	virtual glm::vec3 get_position() const;
	virtual void set_position(glm::vec3 pos);
	virtual glm::quat get_rotation() const;
	virtual void set_rotation(glm::quat rot);

	virtual void set_linear_velocity(glm::vec3 velo);
	virtual void apply_impulse(glm::vec3 force, glm::vec3 rel = {0.0f, 0.0f, 0.0f});

protected:
	void init_rigidbody(glm::vec3 pos);
	virtual btRigidBody* setup_rigid_body(const btTransform& trans) = 0;
	std::unique_ptr<btRigidBody> rigid_body;
};

class MainCharacter : public RigidCharacter {
public:
	MainCharacter();

	Camera& get_camera() { return *camera; }
	virtual void draw(Renderer& renderer) override {}

private:
	virtual btRigidBody* setup_rigid_body(const btTransform& trans);
	virtual AnimationModel* load_model() const override { return nullptr; }

	std::unique_ptr<Camera> camera;
    std::unique_ptr<CameraMotionState> motion_state;
};

class SkeletonCharacter : public RigidCharacter {
private:
    static PModel _prepare_model();
    virtual AnimationModel* load_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);
	virtual btRigidBody* setup_rigid_body(const btTransform& trans);

public:
    SkeletonCharacter(glm::vec3 pos);

	virtual void update(float dt);
};

class TrapItem : public StaticCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* load_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);

public:
	TrapItem(glm::vec3 pos);
};

class ChestTrapItem : public RigidCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* load_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);
	virtual btRigidBody* setup_rigid_body(const btTransform& trans);

public:
	ChestTrapItem(glm::vec3 pos);
};

class TorchItem : public RigidCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* load_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);
	virtual btRigidBody* setup_rigid_body(const btTransform& trans);

public:
	TorchItem(glm::vec3 pos);
};

class BarrelItem : public RigidCharacter {
private:
	static PModel _prepare_model();
	virtual AnimationModel* load_model() const override;
	virtual void intrinsic_transform(Renderer& renderer);
	virtual btRigidBody* setup_rigid_body(const btTransform& trans);

public:
	BarrelItem(glm::vec3 pos);
};

#endif
