//
// Created by jimx on 16-12-17.
//

#ifndef DSPROJECT_ANIMATION_MANAGER_H
#define DSPROJECT_ANIMATION_MANAGER_H

#include "singleton.h"
#include "animation.h"

#include <memory>
#include <list>

class AnimationState {
private:
    BaseAnimation* animation;
    float animation_time_sec;

public:
    AnimationState(BaseAnimation* animation) {
        this->animation = animation;
        animation_time_sec = 0.0f;
    }

    void update(float dt) {
        animation_time_sec += dt;
        animation->update_animation(animation_time_sec);
    }
};

using PAnimationState = std::shared_ptr<AnimationState>;

class AnimationManager : public Singleton<AnimationManager> {
public:
    PAnimationState add_animation(BaseAnimation* animation);
    void cancel_animation(PAnimationState state);

    void update(float dt);
private:
    std::list<PAnimationState> states;
};


#define ANIMATION_MANAGER AnimationManager::get_singleton()

#endif //DSPROJECT_ANIMATION_MANAGER_H

