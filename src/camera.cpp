//
// Created by dollars on 16-12-19.
//

#include "camera.h"

const GLfloat Camera::DEFAULT_YAW = -90.0f;
const GLfloat Camera::DEFAULT_PITCH = 0.0f;
const GLfloat Camera::DEFAULT_SPEED = 1.5f;
const GLfloat Camera::DEFAULT_SENSITIVITY = 0.06f;
const GLfloat Camera::DEFAULT_ZOOM = 45.0f;

Camera::Camera(GLfloat pos_x, GLfloat pos_y, GLfloat pos_z, GLfloat world_up_x, GLfloat world_up_y, GLfloat world_up_z, GLfloat upitch, GLfloat uyaw)
    :front(glm::vec3(0.0f,0.0f,-1.0f)), zoom(DEFAULT_ZOOM), mouse_sensitivity(DEFAULT_SENSITIVITY), movement_speed(DEFAULT_SPEED)
{
    this->position = glm::vec3(pos_x,pos_y,pos_z);
    this->world_up = glm::vec3(world_up_x,world_up_y,world_up_z);
    this->yaw = uyaw;
    this->pitch = upitch;
    this->update_camera_vectors();
}

glm::mat4 Camera::get_view_matrix() const
{
    return glm::lookAt(this->position, this->position + this->front, this->up);
}

void Camera::processkeyboard(Direction udirection, GLfloat delta_time)
{
    GLfloat velocity = delta_time * movement_speed;
    switch (udirection)
    {
    case FORWARD:
        this->position += this->front * velocity;
        break;
    case BACK:
        this->position -= this->front * velocity;
        break;
    case RIGHT:
        this->position -= this->right * velocity;
        break;
    case LEFT:
        this->position += this->right * velocity;
        break;
    default:
        break;
    }
    this->position.y = 1.5f;
    update_camera_vectors();
}

glm::vec3 Camera::get_linear_velocity(Direction udirection, GLfloat delta_time)
{
    GLfloat velocity = delta_time * movement_speed;
    glm::vec3 result;
    switch (udirection)
    {
    case FORWARD:
        result = this->front * velocity;
        break;
    case BACK:
        result = -this->front * velocity;
        break;
    case RIGHT:
        result = -this->right * velocity;
        break;
    case LEFT:
        result = this->right * velocity;
        break;
    default:
        break;
    }
    result[1] = 0.0f;
    return result;
}

void Camera::processmouse(GLfloat x_offset, GLfloat y_offset, GLboolean over_pitch)
{
    x_offset *= mouse_sensitivity;
    y_offset *= mouse_sensitivity;

    this->pitch += y_offset;
    this->yaw += x_offset;


    if(over_pitch)
    {
        if(pitch > 89.0f) pitch = 89.0f;
        else if(pitch < -89.0f) pitch = -89.0f;
    }

    this->update_camera_vectors();

}
void Camera::processmousescroll(GLfloat y_offset)
{
    if (this->zoom >= 1.0f && this->zoom <= 45.0f)
        this->zoom -= y_offset;
    if (this->zoom <= 1.0f)
        this->zoom = 1.0f;
    if (this->zoom >= 45.0f)
        this->zoom = 45.0f;
}
void Camera::update_camera_vectors()
{
    glm::vec3 n_front;
    n_front.x = cos(glm::radians(this->pitch)) * cos(glm::radians(this->yaw));
    n_front.y = sin(glm::radians(this->pitch));
    n_front.z = cos(glm::radians(this->pitch)) * sin(glm::radians(this->yaw));

    this->front = glm::normalize(n_front);
    this->right = -glm::normalize(glm::cross(this->front, this->world_up));
    this->up = glm::normalize(glm::cross(this->front, this->right));
}
