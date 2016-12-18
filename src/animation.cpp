#include "animation.h"
#include "exception.h"
#include "animation_manager.h"
#include "animation_model.h"

#include <glm/gtc/type_ptr.hpp>

using namespace std;

AnimationModel::AnimationModel(PModel model)
{
    this->model = model;
    current_animation = nullptr;
}

void AnimationModel::start_animation(InternString name)
{
    if (current_animation) {
        stop_animation();
    }

    aiAnimation* animation = model->get_animation(name);
    if (!animation) {
        THROW_EXCEPT(E_INVALID_PARAM, "Model::start_animation()", "no such animation '" + string(name.c_str()) + "'");
    }

    current_animation = animation;
    animation_state = ANIMATION_MANAGER.add_animation(this);
}

void AnimationModel::stop_animation()
{
    if (!current_animation) return;

    current_animation = nullptr;
    ANIMATION_MANAGER.cancel_animation(animation_state);
    animation_state = nullptr;
    animation_time_sec = 0;
}

GLfloat identity_transforms[4 * 4 * ShaderProgram::MAX_BONE_TRANSFORMS];

void AnimationModel::draw(Renderer& renderer)
{
    static bool first = true;
    if (first) {
        for (int i = 0; i < ShaderProgram::MAX_BONE_TRANSFORMS; i++) {
            memcpy(identity_transforms + 16 * i, glm::value_ptr(glm::mat4()), 4 * 4 * sizeof(GLfloat));
        }
    }

    vector<Mesh>& meshes = model->get_meshes();

    for(GLuint i = 0; i < meshes.size(); i++){
        Mesh& mesh = meshes[i];

        if (current_animation) {
            vector<glm::mat4> transforms;
            mesh.update_bone_transform(current_animation, animation_time_sec, transforms);

            GLfloat* matrices = new GLfloat[4 * 4 * transforms.size()];
            for (int i = 0; i < transforms.size(); i++) {
                memcpy(matrices + 16 * i, glm::value_ptr(transforms[i]), 4 * 4 * sizeof(GLfloat));
            }
            RENDERER.uniform(ShaderProgram::BONE_TRANSFORMS, transforms.size(), false, matrices);
            delete[] matrices;
        } else {
            RENDERER.uniform(ShaderProgram::BONE_TRANSFORMS, ShaderProgram::MAX_BONE_TRANSFORMS, false, identity_transforms);
        }

        meshes[i].draw(renderer);
    }
}
