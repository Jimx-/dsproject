// Local Headers
#include "config.h"
#include "log_manager.h"
#include "animation_manager.h"
#include "character_manager.h"
#include "particle_system.h"
#include "exception.h"
#include "map.h"
#include "simulation.h"
#include "text_overlay.h"
#include "controllers.h"
#include "gui.h"

#include "characters.h"

// System Headers
#include <GLFW/glfw3.h>

using namespace std;

const string config_file = "dsproject.json";
GLFWwindow* g_window;
Controller* current_controller = nullptr;
map<string, Controller*> controllers;

void load_config()
{
    ConfigFile conf;

    conf.load(config_file);

    std::shared_ptr<Json::Value> root = conf.get_root();
    Json::Value general_config = root->get("general", Json::Value::null);
    Json::Value graphics_config = root->get("graphics", Json::Value::null);

	/* general */
	if (!general_config.isNull()) {
		string difficulty = general_config.get("difficulty", "normal").asString();
		if (difficulty == "easy") g_difficulty = MapGenerator::Difficulty::Easy;
		else if (difficulty == "normal") g_difficulty = MapGenerator::Difficulty::Normal;
		else if (difficulty == "hard") g_difficulty = MapGenerator::Difficulty::Difficult;
        else THROW_EXCEPT(E_INVALID_PARAM, "load_config()", "Bad difficlty argument '" + difficulty + "'");

		g_map_width = general_config.get("map_width", "0").asInt();
		g_map_height = general_config.get("map_height", "0").asInt();
		if (g_map_width <= 0 || g_map_height <= 0)
			THROW_EXCEPT(E_INVALID_PARAM, "load_config()", "Bad map size argument");
	}

    /* graphics */
    if (!graphics_config.isNull()) {
        /* video mode */
        string video_mode = graphics_config.get("video_mode", "800x600").asString();
        size_t cross_pos = video_mode.find('x', 0);
        if (cross_pos == string::npos) {
            THROW_EXCEPT(E_INVALID_PARAM, "load_config()", "Bad video mode argument '" + video_mode + "'");
        }
        string width = video_mode.substr(0, cross_pos);
        string height = video_mode.substr(cross_pos + 1, video_mode.length() - cross_pos - 1);
        g_screen_width = StringUtils::parse_int(width, 800);
        g_screen_height = StringUtils::parse_int(height, 600);

		/* fullscreen */
		string fullscreen = graphics_config.get("fullscreen", "false").asString();
		g_fullscreen = (fullscreen == "true");

		g_MSAA = graphics_config.get("MSAA", "0").asInt();

        g_font = graphics_config.get("font", "DejaVuSerif").asString();
    }
}

void setup_context()
{
    if (!LogManager::get_singleton_ptr()) {
        new LogManager();
        LOG.add_log("dsproject.log", LogMessageLevel::DEBUG, true);
    }

    load_config();

    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	if (g_MSAA > 0) {
		glfwWindowHint(GLFW_SAMPLES, g_MSAA);
	}

    // Create a GLFWwindow object that we can use for GLFW's functions
	GLFWmonitor* monitor = nullptr;
	if (g_fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}
    g_window = glfwCreateWindow(g_screen_width, g_screen_height, "dsproject", monitor, nullptr);
    glfwMakeContextCurrent(g_window);

    // Load OpenGL library
    gladLoadGL();

    new Renderer();
    RENDERER.set_viewport(g_screen_width, g_screen_height);

    new AnimationManager();
    new Simulation();
    new CharacterManager();
	new ParticleSystem();

    TextOverlay::setup_font(g_font);
    GUIWidget::setup_gui();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    current_controller->handle_key(key, scancode, action, mode);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    current_controller->handle_mouse(xpos, ypos);
}

void show_stat(float dt)
{
    static std::shared_ptr<TextOverlay> text(new TextOverlay("", 0.0f, 0.0f, {1.0f, 1.0f, 1.0f}, 0.4));
    static char stats[1000];

    auto pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
    sprintf(stats, "fps: %d, x = %f, y = %f, z = %f", (int) (1 / dt), pos[0], pos[1], pos[2]);

    text->set_text(stats);
    text->set_y(g_screen_height - 20);

    RENDERER.enqueue_overlay(text);
}

void Controller::switch_controller(const string& name)
{
    if (current_controller) current_controller->exit();
    current_controller = controllers[name];
    current_controller->enter();
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
    setup_context();
    LOG.info("Starting game...");

	g_map = new Map(g_map_width, g_map_height);
    controllers["main_menu"] = new MainMenuController();
    controllers["game"] = new GameController();
    controllers["in_game_menu"] = new InGameMenuController();
    controllers["end_game"] = new EndGameController();
    Controller::switch_controller("main_menu");
    //POverlay label(new GUILabel(500.f, 500.f, 500.f, 100.f, "Start", MaterialTexture::create_texture("button_normal.png")));

    double last_time = glfwGetTime();
    double current_time;
    glfwSetWindowTitle(g_window, "dsproject");
    glfwSetKeyCallback(g_window, key_callback);
    glfwSetCursorPosCallback(g_window, mouse_callback);

    // Game loop
	int step = 0;
    while (!glfwWindowShouldClose(g_window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        if (glfwGetMouseButton(g_window, GLFW_MOUSE_BUTTON_LEFT)) {
            double xpos, ypos;
            glfwGetCursorPos(g_window, &xpos, &ypos);
            mouse_callback(g_window, xpos, ypos);
        }

        current_time = glfwGetTime();
        float dt = (float) current_time - (float) last_time;
        last_time = current_time;
        ANIMATION_MANAGER.update(dt);
		PARTICLE_SYSTEM.update(dt);
        SIMULATION.update(dt);
		CHARACTER_MANAGER.update(dt);

        RENDERER.begin_frame();
        if (dt > 0) {
            show_stat(dt);
        }
        current_controller->update_view(RENDERER);
        RENDERER.end_frame();

        glfwSwapBuffers(g_window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}
