// Local Headers
#include "config.h"
#include "mesh.h"
#include "log_manager.h"
#include "renderer.h"
#include "exception.h"

// System Headers
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

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
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
    setup_context();
    LOG.info("Starting game...");

    //loadModel
    Model ourModel("resources/models/skeleton.FBX");
    ourModel.load_animation("onehand_walk", "resources/animations/skeleton_onehand_walk.FBX");
    ourModel.load_animation("onehand_attack", "resources/animations/skeleton_onehand_attack.FBX");
    ourModel.load_animation("onehand_idle", "resources/animations/skeleton_onehand_idle.FBX");
    ourModel.start_animation("onehand_walk");

    RENDERER.use_shader(Renderer::BONE_ANIM_SHADER);

    float angle = 0.0f;
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw the triangle
        angle += 0.01f;
        ourModel.update_animation(0.01f);

        glm::mat4 model;
        //model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));	// It's a bit too big for our scene, so scale it down
        RENDERER.uniform(ShaderProgram::MODEL, 1, GL_FALSE, glm::value_ptr(model));
        glm::mat4 view;
        view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        RENDERER.uniform(ShaderProgram::VIEW, 1, GL_FALSE, glm::value_ptr(view));
        glm::mat4 projection;
        projection = glm::perspective(45.0f, g_screen_width / (GLfloat) g_screen_height, 0.1f, 100.0f);
        RENDERER.uniform(ShaderProgram::PROJECTION, 1, GL_FALSE, glm::value_ptr(projection));

        ourModel.draw();
        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

