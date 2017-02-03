// Local Headers
#include "config.h"
#include "log_manager.h"
#include "animation_manager.h"
#include "character_manager.h"
#include "renderer.h"
#include "animation_model.h"
#include "exception.h"
#include "map.h"

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
    Json::Value graphics_config = root->get("graphics", Json::Value::null);

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
    new CharacterManager();
}

Camera camera(10.0f, 3.0f, 10.0f, 0.0f, 1.0f, 0.0f);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    Camera::Direction dir;
    if (key == GLFW_KEY_W) {
        dir = Camera::Direction::FORWARD;
    }
    if (key == GLFW_KEY_S) {
        dir = Camera::Direction::BACK;
    }
    if (key == GLFW_KEY_A) {
        dir = Camera::Direction::LEFT;
    }
    if (key == GLFW_KEY_D) {
        dir = Camera::Direction::RIGHT;
    }
    if (action != GLFW_RELEASE)
        camera.processkeyboard(dir, 0.1f);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    static float last_x, last_y;
    camera.processmouse(xpos - last_x, -(ypos - last_y), true);
    last_x = xpos;
    last_y = ypos;
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
    setup_context();
    LOG.info("Starting game...");

    //loadModel
    PModel ourModel(new Model("resources/models/skeleton.FBX"));
    ourModel->load_animation("onehand_walk", "resources/animations/skeleton_onehand_walk.FBX");
    ourModel->load_animation("onehand_attack", "resources/animations/skeleton_onehand_attack.FBX");
    ourModel->load_animation("onehand_idle", "resources/animations/skeleton_onehand_idle.FBX");

    PRenderable map(new Map(30, 30));

    int N = 12;
    vector<AnimationModel*> models;
    for (int i = 0; i < N; i++) {
        models.push_back(new AnimationModel(ourModel));
        models[i]->start_animation("onehand_attack");
    }

    float angle = -90.0f;
    double last_time = glfwGetTime();
    double current_time;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
	/*for (int i = 0; i < 32; i++) {
		RENDERER.add_light(glm::vec3((float)4 * ((i / 3) - 1.5f), 1.0f, (float)4 * (i % 3) + 0.3f), glm::vec3(1.0f, 0.8f, 0.5f), 0.4f, 0.5f);
	} */
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


        if (dt > 0) {
            char title[100];
            auto pos = camera.get_position();
            sprintf(title, "Weeaboo [fps: %d, X = %f, Z = %f]", (int) (1 / dt), pos[0], pos[2]);
            glfwSetWindowTitle(window, title);
        }

        angle += 0.1f;
        //camera.set_yaw(angle);
        RENDERER.begin_frame();
        RENDERER.update_camera(camera);

        //map.draw(RENDERER);
        RENDERER.enqueue_renderable(map);
        CHARACTER_MANAGER.submit(RENDERER);

        /*for (int i = 0; i < N; i++) {
            RENDERER.push_matrix();
            RENDERER.translate((float)4 * ((i / 3) - 1.5f) + 10.0f, 0.0f, (float) 4 * (i % 3) + 10.f);
            //RENDERER.rotate(angle, 0.0f, 1.0f, 0.0f);
            RENDERER.scale(0.02f, 0.02f, 0.02f);
            models[i]->draw(RENDERER);
            RENDERER.pop_matrix();
        }*/

        RENDERER.end_frame();

        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

