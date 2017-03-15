#include "controllers.h"
#include "character_manager.h"
#include "particle_system.h"
#include "config.h"

GameController::GameController()
{
    map.reset(g_map);
}

void GameController::update_view(Renderer& renderer)
{
    RENDERER.update_camera(CHARACTER_MANAGER.main_char().get_camera());

    RENDERER.enqueue_renderable(map);
    CHARACTER_MANAGER.submit(RENDERER);
    PARTICLE_SYSTEM.submit(RENDERER);
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
