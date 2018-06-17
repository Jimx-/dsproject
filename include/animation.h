#ifndef DSPROJECT_ANIMATION_H
#define DSPROJECT_ANIMATION_H

#include "mesh.h"
#include "renderable.h"
#include "intern_string.h"

#include <memory>

class BaseAnimation : public Renderable {
public:
    BaseAnimation()
    {
        animation_time_sec = 0.0f;
    }

    virtual void update_animation(float animation_time)
    {
        animation_time_sec = animation_time;
    }

protected:
    float animation_time_sec;
};

using PAnimation = std::shared_ptr<BaseAnimation>;

#endif

