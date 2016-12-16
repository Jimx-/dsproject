// Local Headers
#include "shader_program.h"
#include "mesh.h"
#include "log_manager.h"
#include "renderer.h"

// System Headers
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

// Window dimensions
const GLuint WIDTH = 1366, HEIGHT = 768;

GLFWwindow* window;

void setup_context()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow(WIDTH, HEIGHT, "Weeaboo", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Load OpenGL library
    gladLoadGL();

    if (!LogManager::get_singleton_ptr()) {
        new LogManager();
        LOG.add_log("weeaboo.log", LogMessageLevel::DEBUG, true);
    }

    new Renderer();
}

// The MAIN function, from here we start the application and run the game loop
int main()
{
    setup_context();
    LOG.info("Starting game...");

    //loadModel
    Model ourModel("resources/models/skeleton.FBX");

    RENDERER.use_shader(Renderer::LIGHTING_SHADER);
    RENDERER.uniform(ShaderProgram::DIFFUSE_TEXTURE, 0);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.3f, 1.0f);
    glClearDepth(1.0f);
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
        RENDERER.use_shader(Renderer::LIGHTING_SHADER);

        //loadModel
        angle += 0.01f;
        glm::mat4 model;
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.6f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));	// It's a bit too big for our scene, so scale it down
        RENDERER.uniform(ShaderProgram::MODEL, 1, GL_FALSE, glm::value_ptr(model));
        glm::mat4 view;
        view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        RENDERER.uniform(ShaderProgram::VIEW, 1, GL_FALSE, glm::value_ptr(view));
        glm::mat4 projection;
        projection = glm::perspective(45.0f, WIDTH / (GLfloat) HEIGHT, 0.1f, 100.0f);
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





