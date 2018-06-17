#ifndef DSPROJECT_MOTION_STATE_H
#define DSPROJECT_MOTION_STATE_H

#include "camera.h"

#include <btBulletDynamicsCommon.h>

class CameraMotionState : public btMotionState {
protected:
    Camera* camera;
    btTransform mInitialPosition;

public:
    CameraMotionState(const btTransform &initialPosition, Camera* cam);
    virtual ~CameraMotionState();

    virtual void getWorldTransform(btTransform &worldTrans) const;
    virtual void setWorldTransform(const btTransform &worldTrans);
};

#endif
