// Local Headers
#include "config.h"
#include "log_manager.h"
#include "animation_manager.h"
#include "character_manager.h"
#include "particle_system.h"
#include "renderer.h"
#include "animation_model.h"
#include "exception.h"
#include "map.h"
#include "particle.h"
#include "simulation.h"

#include "characters.h"

// System Headers
#include <GLFW/glfw3.h>

using namespace std;

const string config_file = "weeaboo.json";
GLFWwindow* window;

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
    }
}

void setup_context()
{
    if (!LogManager::get_singleton_ptr()) {
        new LogManager();
        LOG.add_log("weeaboo.log", LogMessageLevel::DEBUG, true);
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
    window = glfwCreateWindow(g_screen_width, g_screen_height, "Weeaboo", monitor, nullptr);
    glfwMakeContextCurrent(window);

    // Load OpenGL library
    gladLoadGL();

    new Renderer();
    RENDERER.set_viewport(g_screen_width, g_screen_height);

    new AnimationManager();
    new Simulation();
    new CharacterManager();
	new ParticleSystem();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
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
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static float last_x, last_y;
    CHARACTER_MANAGER.main_char().get_camera().processmouse(xpos - last_x, -(ypos - last_y), true);
    last_x = xpos;
    last_y = ypos;
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
    setup_context();
    LOG.info("Starting game...");

	g_map = new Map(g_map_width, g_map_height);
    PRenderable map(g_map);

    double last_time = glfwGetTime();
    double current_time;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Game loop
	int step = 0;
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        current_time = glfwGetTime();
        float dt = (float) current_time - (float) last_time;
        last_time = current_time;
        ANIMATION_MANAGER.update(dt);
		PARTICLE_SYSTEM.update(dt);
        SIMULATION.update(dt);
		CHARACTER_MANAGER.update(dt);

        if (dt > 0) {
            char title[100];
            auto pos = CHARACTER_MANAGER.main_char().get_camera().get_position();
            sprintf(title, "Weeaboo [fps: %d, X = %f, Y = %f, Z = %f]", (int) (1 / dt), pos[0], pos[1], pos[2]);
            glfwSetWindowTitle(window, title);
        }

        RENDERER.begin_frame();
        RENDERER.update_camera(CHARACTER_MANAGER.main_char().get_camera());

        RENDERER.enqueue_renderable(map);
        CHARACTER_MANAGER.submit(RENDERER);
		PARTICLE_SYSTEM.submit(RENDERER);

        RENDERER.end_frame();

        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}
