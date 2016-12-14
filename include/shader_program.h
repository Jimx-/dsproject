//
// Created by dollars on 16-12-13.
//

#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <glad/glad.h>

class ShaderProgram {
public:
    ShaderProgram(const char * vertexPath, const char* fragmentPath);
    GLuint get_program() const { return program; }

    void use();
private:
    GLuint program;
};


#endif // SHADERPROGRAM_H

