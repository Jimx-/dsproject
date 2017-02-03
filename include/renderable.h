#ifndef WEEABOO_RENDERABLE_H
#define WEEABOO_RENDERABLE_H

#include "renderer.h"

#include <memory>

class Renderable {
public:
    virtual void draw(Renderer& renderer) = 0;
};

using PRenderable = std::shared_ptr<Renderable>;

#endif

