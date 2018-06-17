#ifndef DSPROJECT_RENDERABLE_H
#define DSPROJECT_RENDERABLE_H

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

class Overlay : public Renderable {
public:
	enum class Technique {
		TEXT,
		GUI_ELEMENT,
	};

	Overlay(Technique tech = Technique::GUI_ELEMENT) : tech(tech) { }
    Technique get_technique() const { return tech; }

private:
	Technique tech;
};

using PRenderable = std::shared_ptr<Renderable>;
using POverlay = std::shared_ptr<Overlay>;

#endif

