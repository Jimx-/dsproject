#ifndef DSPROJECT_GUI_H
#define DSPROJECT_GUI_H

#include "renderable.h"
#include "text_overlay.h"
#include "material.h"

#include <string>
#include <functional>

class GUIWidget : public Overlay {
public:
    using OnClickListener = std::function<void(GUIWidget*)>;

    GUIWidget(float left, float top, float width, float height);

    static void setup_gui();

    float get_width() const { return width; }
    float get_height() const { return height; }

    virtual void set_width(float w) { width = w; }
    virtual void set_height(float h) { height = h; }
    virtual void set_left(float l) { left = l; }
    virtual void set_top(float t) { top = t; }

    virtual void set_enabled(bool enable) { }
    virtual void handle_mouse(double xpos, double ypos) { }
    virtual void set_on_click_listener(OnClickListener listener) { }

protected:
    float left, top, width, height;

    bool hit_test(double xpos, double ypos);
};

using PGUIWidget = std::shared_ptr<GUIWidget>;

class GUIPlaceholder : public GUIWidget {
public:
    GUIPlaceholder(float width, float height) : GUIWidget(0.0f, 0.0f, width, height) { }
    void draw(Renderer& renderer) override { }
};

class GUILabel : public GUIWidget {
public:
    GUILabel(float left, float top, float width, float height, const std::string& text, PMaterialTexture background = nullptr);
    GUILabel(float width, float height, const std::string& text, PMaterialTexture background = nullptr);
    GUILabel(const std::string& text, float font_size = 1.0f, glm::vec3 color = {1.0f, 1.0f, 1.0f}, PMaterialTexture background = nullptr);

    void draw(Renderer& renderer) override;

    virtual void set_left(float l);
    virtual void set_top(float t);
    virtual void set_mask_width(float mw) { mask_width = mw; }
    void set_text(const std::string& text) { this->text->set_text(text); }
    void set_color(glm::vec3 color) { this->text->set_color(color); }

    virtual void set_enabled(bool enable) override {
        this->enabled = enable;
    }
    virtual void set_on_click_listener(OnClickListener listener) { this->on_click_listener = listener; }

    virtual void handle_mouse(double xpos, double ypos) override;
private:
    std::shared_ptr<TextOverlay> text;
    PMaterialTexture background;

    float mask_width;

    bool enabled;
    enum {
        MOUSE_OUTSIDE,
        MOUSE_HOVER,
        MOUSE_PRESSED,
    } mouse_state;
    OnClickListener on_click_listener;

    glm::vec3 saved_color;
};

#endif
