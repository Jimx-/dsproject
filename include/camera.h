//
// Created by dollars on 16-12-19.
//

#ifndef GLITTER_CAMERA_H
#define GLITTER_CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    enum Direction{
        FORWARD,
        BACK,
        LEFT,
        RIGHT,
    };

    const static GLfloat DEFAULT_YAW;
#undef DEFAULT_PITCH
    const static GLfloat DEFAULT_PITCH;
    const static GLfloat DEFAULT_SPEED;
    const static GLfloat DEFAULT_SENSITIVITY;
    const static GLfloat DEFAULT_ZOOM;


    //set the look_at with values : position x y z  world_up x y z, yaw, pitch,
    Camera(GLfloat pos_x, GLfloat pos_y, GLfloat pos_z, GLfloat world_up_x, GLfloat world_up_y, GLfloat world_up_z, GLfloat upitch = DEFAULT_PITCH, GLfloat uyaw = DEFAULT_YAW);

    // return look_at matrix
    glm::mat4 get_view_matrix() const;

    GLfloat get_yaw() const { return yaw; }
    void set_yaw(GLfloat yaw) { this->yaw = yaw; update_camera_vectors(); }
    void set_pitch(GLfloat pitch) { this->pitch = pitch; update_camera_vectors(); }

    glm::vec3 get_position() const { return position; }
    void set_position(glm::vec3 pos) { this->position = pos; update_camera_vectors(); }

    glm::vec3 get_linear_velocity(Direction udirection, GLfloat delta_time);
    // the change of position through keyboard
    void processkeyboard(Direction udirection, GLfloat delta_time);

    // the change of position through mouse
    void processmouse(GLfloat x_offset, GLfloat y_offset, GLboolean over_pitch);

    // zoom
    void processmousescroll(GLfloat y_offset);
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 world_up;
    glm::vec3 right;
    glm::vec3 up;

    GLfloat yaw;
    GLfloat pitch;
    GLfloat zoom;

    GLfloat movement_speed;
    GLfloat mouse_sensitivity;

    void update_camera_vectors();
};


#endif //GLITTER_CAMERA_H
