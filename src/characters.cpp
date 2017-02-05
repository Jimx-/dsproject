#include "log_manager.h"
#include "characters.h"
#include "map.h"

BaseCharacter::BaseCharacter(glm::vec3 pos) : pos(pos)
{

}

void BaseCharacter::init_model()
{
    model = get_model();
}

void BaseCharacter::draw(Renderer& renderer)
{
    renderer.push_matrix();
    renderer.translate(pos[0] * Map::TILE_SIZE, 0.0f, pos[2] * Map::TILE_SIZE);
	intrinsic_transform(renderer);
    model->draw(renderer);
    renderer.pop_matrix();
}

SkeletonCharacter::SkeletonCharacter(glm::vec3 pos) : BaseCharacter(pos)
{
    init_model();
    model->start_animation("idle");
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

AnimationModel* SkeletonCharacter::get_model() const
{
    return new AnimationModel(_prepare_model());
}

void SkeletonCharacter::intrinsic_transform(Renderer& renderer) 
{
	renderer.scale(0.02f, 0.02f, 0.02f);
}

TrapItem::TrapItem(glm::vec3 pos) : BaseCharacter(pos)
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

AnimationModel* TrapItem::get_model() const
{
	return new AnimationModel(_prepare_model());
}

void TrapItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(0.5f * Map::TILE_SIZE, 0.0f, 0.5f * Map::TILE_SIZE);
	renderer.rotate(glm::radians(-90.0f), 1.0f, 0.0f, 0.0f);
	renderer.scale(0.45f, 0.45f, 0.45f);
}

ChestTrapItem::ChestTrapItem(glm::vec3 pos) : BaseCharacter(pos)
{
	init_model();
}

PModel ChestTrapItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/MedievalChest_01_open.fbx"));
	return ourModel;
}

AnimationModel* ChestTrapItem::get_model() const
{
	return new AnimationModel(_prepare_model());
}

void ChestTrapItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(-0.5f * Map::TILE_SIZE, 0.0f, 0.5f * Map::TILE_SIZE);
	renderer.scale(0.04f, 0.04f, 0.04f);
}

TorchItem::TorchItem(glm::vec3 pos) : BaseCharacter(pos)
{
	init_model();
}

PModel TorchItem::_prepare_model()
{
	static PModel ourModel(new Model("resources/models/torch4.obj"));
	return ourModel;
}

AnimationModel* TorchItem::get_model() const
{
	return new AnimationModel(_prepare_model());
}

void TorchItem::intrinsic_transform(Renderer& renderer)
{
	renderer.translate(0.5f * Map::TILE_SIZE, 0.0f, 0.5f * Map::TILE_SIZE);
	renderer.scale(0.18f, 0.18f, 0.18f);
}
