#include "motion_state.h"

CameraMotionState::CameraMotionState(const btTransform &initialPosition, Camera* cam)
{
    camera = cam;
    mInitialPosition = initialPosition;
}

CameraMotionState::~CameraMotionState()
{
}

void CameraMotionState::getWorldTransform(btTransform &worldTrans) const
{
    worldTrans = mInitialPosition;
}

void CameraMotionState::setWorldTransform(const btTransform &worldTrans)
{
    if(camera == nullptr)
        return; // silently return before we set a node

    btVector3 pos = worldTrans.getOrigin();
    camera->set_position(glm::vec3{pos.x(), pos.y(), pos.z()});
}
