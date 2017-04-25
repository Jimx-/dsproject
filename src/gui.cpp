#include "gui.h"
#include "config.h"
#include "log_manager.h"

static GLuint gui_VAO;
static GLuint gui_VBO;

void GUIWidget::setup_gui()
{
    glGenVertexArrays(1, &gui_VAO);
    glGenBuffers(1, &gui_VBO);
    glBindVertexArray(gui_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, gui_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GUIWidget::GUIWidget(float left, float top, float width, float height) : Overlay(Overlay::Technique::GUI_ELEMENT),
                                                                         left(left), top(top), width(width), height(height)
{
}

bool GUIWidget::hit_test(double xpos, double ypos)
{
    return ((float) xpos >= left && (float) xpos < left + width
        && (float) ypos <= top && (float) ypos > top - height);
}

GUILabel::GUILabel(float left, float top, float width, float height, const std::string& caption, PMaterialTexture background) :
    GUIWidget(left, top, width, height), background(background), on_click_listener(nullptr)
{
    text.reset(new TextOverlay(caption, 0.0, 0.0));
    auto text_size = text->size_hint();
    text->set_x((width - text_size[0]) / 2 + left);
    text->set_y(top - (height - text_size[1]) / 2 - text_size[1]);
    mouse_state = MOUSE_OUTSIDE;
    enabled = false;
    mask_width = 1.0f;
}

GUILabel::GUILabel(float width, float height, const std::string& text,
                   PMaterialTexture background) : GUILabel(0.0f, 0.0f, width, height, text, background)
{
    mouse_state = MOUSE_OUTSIDE;
    enabled = false;
    mask_width = 1.0f;
}

GUILabel::GUILabel(const std::string& caption, float font_size, glm::vec3 color, PMaterialTexture background) :
                    GUIWidget(0.0f, 0.0f, 0.0f, 0.0f), background(background), on_click_listener(nullptr)
{
    text.reset(new TextOverlay(caption, 0.0, 0.0, color, font_size));
    auto text_size = text->size_hint();
    set_width(text_size[0]);
    set_height(text_size[1]);
    mouse_state = MOUSE_OUTSIDE;
    enabled = false;
    mask_width = 1.0f;
}

void GUILabel::set_left(float l)
{
    GUIWidget::set_left(l);
    auto text_size = text->size_hint();
    text->set_x((width - text_size[0]) / 2 + left);
}

void GUILabel::set_top(float t)
{
    GUIWidget::set_top(t);
    auto text_size = text->size_hint();
    text->set_y(top - (height - text_size[1]) / 2 - text_size[1]);
}

void GUILabel::draw(Renderer& renderer)
{
    glm::mat4 proj = glm::ortho(0.0f, (float)g_screen_width, 0.0f, (float)g_screen_height);

    if (background) {
        renderer.use_shader(Renderer::GUI_SHADER);
        renderer.uniform("uProjection", 1, false, glm::value_ptr(proj));

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(gui_VAO);
        glActiveTexture(GL_TEXTURE0);
        background->bind(Renderer::DIFFUSE_TEXTURE_TARGET);

        GLfloat vertices[6][4] = {
            { left,     top,   0.0, 0.0 },
            { left,     top - height,       0.0, 1.0 },
            { left + width * mask_width, top - height,       mask_width, 1.0 },

            { left,     top,   0.0, 0.0 },
            { left + width * mask_width, top - height,       mask_width, 1.0 },
            { left + width * mask_width, top,   mask_width, 0.0 }
        };

        glBindBuffer(GL_ARRAY_BUFFER, gui_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    renderer.use_shader(Renderer::TEXT_OVERLAY_SHADER);
    renderer.uniform("uProjection", 1, false, glm::value_ptr(proj));

    text->draw(renderer);
}

void GUILabel::handle_mouse(double xpos, double ypos)
{
    if (!enabled) return;

    if (!hit_test(xpos, ypos)) {
        if (mouse_state == MOUSE_HOVER) text->set_color(saved_color);
        mouse_state = MOUSE_OUTSIDE;
        return;
    }

    int mouse = glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT);
    if (!mouse) {
        if (mouse_state == MOUSE_PRESSED) {
            mouse_state = MOUSE_HOVER;
        } else if (mouse_state == MOUSE_OUTSIDE) {
            saved_color = text->get_color();
            glm::vec3 new_color(0.8f * saved_color);
            if (saved_color[0] == 0.0f && saved_color[1] == 0.0f && saved_color[2] == 0.0f) new_color = {0.2f, 0.2f, 0.2f};
            text->set_color(new_color);
            mouse_state = MOUSE_HOVER;
        }
    } else {
        if (mouse_state != MOUSE_PRESSED) {
            mouse_state = MOUSE_PRESSED;

            if (on_click_listener) {
                on_click_listener(this);
            }
        }
    }
}
