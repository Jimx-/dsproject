#ifndef WEEABOO_RENDERABLE_H
#define WEEABOO_RENDERABLE_H

#include "renderer.h"

class Renderable {
public:
    virtual void draw(Renderer& renderer) = 0;
};

#endif

