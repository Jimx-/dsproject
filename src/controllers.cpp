#include "controllers.h"
#include "character_manager.h"
#include "particle_system.h"
#include "config.h"
#include "log_manager.h"

#include <cstdlib>
#include <sstream>

GameController::GameController()
{
    map.reset(g_map);
    hpbar.reset(new GUILabel(0.05f * g_screen_width, 0.95f * g_screen_height, 0.6f * g_screen_width, 20.f, "", MaterialTexture::create_texture("hpbar.png")));
}

void GameController::update_view(Renderer& renderer)
{
    RENDERER.update_camera(CHARACTER_MANAGER.main_char().get_camera());

    RENDERER.enqueue_renderable(map);
    CHARACTER_MANAGER.submit(RENDERER);
    PARTICLE_SYSTEM.submit(RENDERER);

    hpbar->set_mask_width(CHARACTER_MANAGER.main_char().get_hp() / MainCharacter::MAX_HP);
    RENDERER.enqueue_overlay(hpbar);
}

void GameController::handle_key(int key, int scancode, int action, int mode)
{
    Camera::Direction dir;
    bool move = false;
    if (key == GLFW_KEY_W) {
        dir = Camera::Direction::FORWARD;
        move = true;
    }
    if (key == GLFW_KEY_S) {
        dir = Camera::Direction::BACK;
        move = true;
    }
    if (key == GLFW_KEY_A) {
        dir = Camera::Direction::LEFT;
        move = true;
    }
    if (key == GLFW_KEY_D) {
        dir = Camera::Direction::RIGHT;
        move = true;
    }
    if (action != GLFW_RELEASE) {
        if (move) {
            CHARACTER_MANAGER.main_char().set_linear_velocity(
                CHARACTER_MANAGER.main_char().get_camera().get_linear_velocity(dir, 3.0f)
            );
        }

        if (key == GLFW_KEY_ESCAPE) {
            glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            switch_controller("in_game_menu");
        } else if (key == GLFW_KEY_SPACE) {
            auto pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
            if (pos[1] < 1.26f) {
                CHARACTER_MANAGER.main_char().apply_impulse({0.0f, 4.0f, 0.0f});
            }
        }
    } else {
        if (move) CHARACTER_MANAGER.main_char().set_linear_velocity({0.0f, 0.0f, 0.0f});
    }
}

void GameController::handle_mouse(double xpos, double ypos)
{
    static float last_x, last_y;
    CHARACTER_MANAGER.main_char().get_camera().processmouse(xpos - last_x, -(ypos - last_y), true);
    last_x = xpos;
    last_y = ypos;
}

void GameController::enter()
{
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    RENDERER.toggle_minimap(true);
}

void GameController::exit()
{
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    RENDERER.toggle_minimap(false);
}

WidgetController::WidgetController(const std::string& bg)
{
    background.reset(new GUILabel(0.f, g_screen_height, g_screen_width, g_screen_height,  "", MaterialTexture::create_texture(bg)));
    layout_y = 0.9 * g_screen_height;
}

void WidgetController::add_widget(PGUIWidget widget)
{
    float w = widget->get_width();
    float h = widget->get_height();

    widget->set_top(layout_y);
    layout_y -= h + 50;

    widget->set_left(g_screen_width * 0.3f - w * 0.5f);
    widgets.push_back(widget);
}

void WidgetController::update_view(Renderer& renderer)
{
    renderer.enqueue_overlay(background);
    for (auto&& p: widgets) {
        renderer.enqueue_overlay(p);
    }
}

void WidgetController::handle_key(int key, int scancode, int action, int mode)
{

}

void WidgetController::handle_mouse(double xpos, double ypos)
{
    for (auto&& p: widgets) {
        p->handle_mouse(xpos, g_screen_height - ypos);
    }
}

MainMenuController::MainMenuController() : WidgetController("start2.bmp")
{
    PGUIWidget title(new GUILabel("Weeaboo's", 2.0f, {0.0f, 0.0f, 0.0f}));
    PGUIWidget title2(new GUILabel("Adventure", 2.0f, {0.0f, 0.0f, 0.0f}));
    PGUIWidget split(new GUIPlaceholder(0.0f, 80.0f));
    PGUIWidget btn_start(new GUILabel("Start", 1.5f, {0.0f, 0.0f, 0.0f}));
    PGUIWidget btn_exit(new GUILabel("Exit", 1.5f, {0.0f, 0.0f, 0.0f}));

    btn_start->set_enabled(true);
    btn_start->set_on_click_listener([](GUIWidget*){ Controller::switch_controller("game"); });
    btn_exit->set_enabled(true);
    btn_exit->set_on_click_listener([](GUIWidget*){ ::exit(0); });

    add_widget(title);
    add_widget(title2);
    add_widget(split);
    add_widget(btn_start);
    add_widget(btn_exit);
}

InGameMenuController::InGameMenuController(): WidgetController("start.bmp")
{
    PGUIWidget title(new GUILabel("Paused", 3.0f, {0.0f, 0.0f, 0.0f}));
    PGUIWidget split(new GUIPlaceholder(0.0f, 80.0f));
    PGUIWidget btn_resume(new GUILabel("Resume", 1.5f, {0.0f, 0.0f, 0.0f}));
    PGUIWidget btn_exit(new GUILabel("Exit", 1.5f, {0.0f, 0.0f, 0.0f}));

    btn_resume->set_enabled(true);
    btn_resume->set_on_click_listener([](GUIWidget*){ Controller::switch_controller("game"); });
    btn_exit->set_enabled(true);
    btn_exit->set_on_click_listener([](GUIWidget*){ ::exit(0); });

    add_widget(title);
    add_widget(split);
    add_widget(btn_resume);
    add_widget(btn_exit);
}

EndGameController::EndGameController(): WidgetController("start.bmp")
{
    title.reset(new GUILabel("GAME CLEAR", 1.5f, {0.0f, 0.0f, 0.0f}));
    score.reset(new GUILabel("Score: 9999", 0.5f, {0.0f, 0.0f, 0.0f}));
    PGUIWidget split(new GUIPlaceholder(0.0f, 80.0f));
    PGUIWidget btn_exit(new GUILabel("Exit", 1.2f, {0.0f, 0.0f, 0.0f}));

    btn_exit->set_enabled(true);
    btn_exit->set_on_click_listener([](GUIWidget*){ ::exit(0); });

    add_widget(title);
    add_widget(score);
    add_widget(split);
    add_widget(btn_exit);
}

void EndGameController::enter()
{
    if (CHARACTER_MANAGER.main_char().get_hp() <= 0.0f) {
        title->set_text("YOU DIED");
        title->set_color({1.0f, 0.0f, 0.0f});
    } else {
        title->set_text("GAME CLEAR");
    }
    std::stringstream ss;
    ss << "Score: ";
    ss << CHARACTER_MANAGER.main_char().get_score();
    score->set_text(ss.str());
}
