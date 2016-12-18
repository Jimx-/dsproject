//
// Created by jimx on 16-12-18.
//

#ifndef WEEABOO_LIGHT_H
#define WEEABOO_LIGHT_H

#include <glm/gtc/matrix_transform.hpp>

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float linear;
    float quadratic;

    Light(const glm::vec3& position, const glm::vec3& color, float Ld, float Qd)
    {
        this->position = position;
        this->color = color;
        linear = Ld;
        quadratic = Qd;
    }
};

#endif //WEEABOO_LIGHT_H

