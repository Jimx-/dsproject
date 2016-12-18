// Local Headers
#include "config.h"
#include "log_manager.h"
#include "animation_manager.h"
#include "renderer.h"
#include "animation_model.h"
#include "exception.h"

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
    window = glfwCreateWindow(g_screen_width, g_screen_height, "Weeaboo", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Load OpenGL library
    gladLoadGL();

    new Renderer();
    RENDERER.set_viewport(g_screen_width, g_screen_height);

    new AnimationManager();
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

    int N = 12;
    vector<AnimationModel*> models;
    for (int i = 0; i < N; i++) {
        models.push_back(new AnimationModel(ourModel));
        models[i]->start_animation("onehand_walk");
    }

    float angle = 0.0f;
    double last_time = glfwGetTime();
    double current_time;

    RENDERER.add_light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.5f, 1.0f);
    RENDERER.add_light(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.8f, 0.0f));
    RENDERER.add_light(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.8f, 0.0f));
    // Game loop
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
            sprintf(title, "Weeaboo [fps: %d]", (int) (1 / dt));
            glfwSetWindowTitle(window, title);
        }

        RENDERER.begin_frame();

        for (int i = 0; i < N; i++) {
            RENDERER.push_matrix();
            RENDERER.translate((float)4 * ((i / 3) - 1.5f), -2.0f, (float) -4 * (i % 3));
            RENDERER.rotate(angle, 0.0f, 1.0f, 0.0f);
            RENDERER.scale(0.04f, 0.04f, 0.04f);
            models[i]->draw(RENDERER);
            RENDERER.pop_matrix();
        }

        RENDERER.end_frame();

        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

