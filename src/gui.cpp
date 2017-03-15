#include "gui.h"
#include "config.h"

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

GUILabel::GUILabel(float left, float top, float width, float height, const std::string& caption, PMaterialTexture background) :
    GUIWidget(left, top, width, height), background(background)
{
    text.reset(new TextOverlay(caption, 0.0, 0.0));
    auto text_size = text->size_hint();
    text->set_x((width - text_size[0]) / 2 + left);
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
            { left + width, top - height,       1.0, 1.0 },

            { left,     top,   0.0, 0.0 },
            { left + width, top - height,       1.0, 1.0 },
            { left + width, top,   1.0, 0.0 }
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
