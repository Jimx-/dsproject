//
// Created by jimx on 16-12-18.
//

#ifndef DSPROJECT_MODEL_ANIMATION_H
#define DSPROJECT_MODEL_ANIMATION_H

#include "animation_manager.h"

class AnimationModel : public BaseAnimation {
public:
    AnimationModel(PModel model);

    void start_animation(InternString name);
    void stop_animation();

    void draw(Renderer& renderer);
private:
    PModel model;

    aiAnimation* current_animation;
    std::shared_ptr<AnimationState> animation_state;
};

#endif //DSPROJECT_MODEL_ANIMATION_H

