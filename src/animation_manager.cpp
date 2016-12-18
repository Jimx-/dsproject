//
// Created by jimx on 16-12-17.
//

#include "animation_manager.h"

template <> AnimationManager* Singleton<AnimationManager>::singleton = nullptr;

PAnimationState AnimationManager::add_animation(BaseAnimation* animation)
{
    PAnimationState state(new AnimationState(animation));
    states.push_back(state);

    return state;
}

void AnimationManager::cancel_animation(PAnimationState state)
{
    auto it = states.begin();
    for (; it != states.end(); it++) {
        if ((*it) == state) {
            break;
        }
    }

    if (it != states.end()) {
        states.erase(it);
    }
}

void AnimationManager::update(float dt)
{
    for (auto it = states.begin(); it != states.end(); it++) {
        (*it)->update(dt);
    }
}

