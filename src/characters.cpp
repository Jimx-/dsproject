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
    renderer.scale(get_scale_factor(), get_scale_factor(), get_scale_factor());
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
    ourModel->load_animation("walk", "resources/animations/skeleton_onehand_walk.FBX");
    ourModel->load_animation("attack", "resources/animations/skeleton_onehand_attack.FBX");
    ourModel->load_animation("idle", "resources/animations/skeleton_onehand_idle.FBX");
    return ourModel;
}

AnimationModel* SkeletonCharacter::get_model() const
{
    return new AnimationModel(_prepare_model());
}

