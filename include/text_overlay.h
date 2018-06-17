#ifndef DSPROJECT_TEXT_OVERLAY_H
#define DSPROJECT_TEXT_OVERLAY_H

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

    void set_text(const std::string& text) { this->text = text; }
    void set_x(float x) { this->x = x; }
    void set_y(float y) { this->y = y; }

    glm::vec3 get_color() const { return color; }
    void set_color(glm::vec3 c) { color = c; }

    glm::vec2 size_hint() const;

private:
    std::string text;
    float x, y, scale;
    glm::vec3 color;
};

#endif
