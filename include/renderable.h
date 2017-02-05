#ifndef WEEABOO_RENDERABLE_H
#define WEEABOO_RENDERABLE_H

#include "renderer.h"

#include <memory>

class Renderable {
public:
	Renderable(bool opaque = true) : opaque(opaque) { }
    virtual void draw(Renderer& renderer) = 0;

	bool is_opaque() const { return opaque; }
private:
	bool opaque;
};

using PRenderable = std::shared_ptr<Renderable>;

#endif

