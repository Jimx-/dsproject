#ifndef WEEABOO_GUI_H
#define WEEABOO_GUI_H

#include "renderable.h"
#include "text_overlay.h"
#include "material.h"

#include <string>

class GUIWidget : public Overlay {
public:
    GUIWidget(float left, float top, float width, float height);

    static void setup_gui();

protected:
    float left, top, width, height;
};

class GUILabel : public GUIWidget {
public:
    GUILabel(float left, float top, float width, float height, const std::string& text, PMaterialTexture background = nullptr);

    void draw(Renderer& renderer) override;

private:
    std::shared_ptr<TextOverlay> text;
    PMaterialTexture background;
};

#endif
