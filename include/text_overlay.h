#ifndef WEEABOO_TEXT_OVERLAY_H
#define WEEABOO_TEXT_OVERLAY_H

#include "renderable.h"

#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class TextOverlay : public Overlay {
public:
    static void setup_font(const std::string& name);

    TextOverlay(const std::string& text, float x, float y, glm::vec3 color = {1.0f, 1.0f, 1.0f}, float scale = 1.0f);
    virtual void draw(Renderer& renderer) override;

private:
    std::string text;
    float x, y, scale;
    glm::vec3 color;
};

#endif