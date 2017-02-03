//
// Created by dollars on 16-12-13.
//

#include "shader_program.h"
#include "log_manager.h"
#include "exception.h"

#include <string>
#include <fstream>
#include <sstream>

using namespace std;

ShaderProgram::ShaderProgram(const char * vertexPath, const char* fragmentPath, const char* geometryPath)
{
    string vertexCode;
    string fragmentCode;
    ifstream vShaderFile;
    ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (ifstream::failure e) {
        THROW_EXCEPT(E_FILE_NOT_FOUND, "ShaderProgram::ShaderProgram()", "shader file is not successfully read");
    }
    const GLchar *vShaderCode = vertexCode.c_str();
    const GLchar *fShaderCode = fragmentCode.c_str();

    GLuint vertex, fragment;
    GLint success;
    GLchar infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        THROW_EXCEPT(E_RESOURCE_ERROR, "ShaderProgram::ShaderProgram()", "SHADER::VERTTEX::COMPILATION_FAILED(" + string(infoLog) + ")");
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        THROW_EXCEPT(E_RESOURCE_ERROR, "ShaderProgram::ShaderProgram()", "SHADER::FRAGMENT::COMPILATION_FAILED(" + string(infoLog) + ")");
    }


    this->program = glCreateProgram();
    glAttachShader(this->program, vertex);
    glAttachShader(this->program, fragment);

    GLuint geom;
    if (geometryPath) {
        string geometryCode;
        ifstream gShaderFile;

        gShaderFile.exceptions(std::ifstream::badbit);
        try {
            gShaderFile.open(geometryPath);

            stringstream gShaderStream;

            gShaderStream << gShaderFile.rdbuf();

            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
        catch (ifstream::failure e) {
            THROW_EXCEPT(E_FILE_NOT_FOUND, "ShaderProgram::ShaderProgram()", "shader file is not successfully read");
        }
        const GLchar *gShaderCode = geometryCode.c_str();
        geom = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geom, 1, &gShaderCode, NULL);
        glCompileShader(geom);

        glGetShaderiv(geom, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geom, 512, NULL, infoLog);
            THROW_EXCEPT(E_RESOURCE_ERROR, "ShaderProgram::ShaderProgram()", "SHADER::GEOMETRY::COMPILATION_FAILED(" + string(infoLog) + ")");
        }
        glAttachShader(this->program, geom);
    }

    glLinkProgram(this->program);
    glGetProgramiv(this->program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(this->program, 512, NULL, infoLog);
        THROW_EXCEPT(E_RESOURCE_ERROR, "ShaderProgram::ShaderProgram()", "SHADER::PROGRAM::LINKING_FAILED(" + string(infoLog) + ")");
    }

    int num_uniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);

    for (int i = 0; i < num_uniforms; i++) {
        char name[256] = "";
        int name_length;
        GLint size;
        GLenum type;
        glGetActiveUniform(program, i, sizeof(name), &name_length, &size, &type, name);

        GLint loc = glGetUniformLocation(program, name);

        InternString uni_name(name);
        uniforms[uni_name] = std::make_pair(loc, type);
    }
    unbind();

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometryPath) glDeleteShader(geom);
}

void ShaderProgram::uniform(UniformID id, float v0, float v1, float v2, float v3)
{
    Binding b = get_uniform_binding(id);
    uniform(b, v0, v1, v2, v3);
}

void ShaderProgram::uniform(UniformID id, int i0)
{
    Binding b = get_uniform_binding(id);
    uniform(b, i0);
}

void ShaderProgram::uniform(UniformID id, float f0)
{
    Binding b = get_uniform_binding(id);

    uniform(b, f0);
}

void ShaderProgram::uniform(UniformID id, GLsizei count, GLboolean transpose, const GLfloat* mat)
{
    Binding b = get_uniform_binding(id);
    //cout << id.c_str() << " " << b.first << " " << b.second << endl;
    uniform(b, count, transpose, mat);
}

void ShaderProgram::bind()
{
    glUseProgram(program);
}

void ShaderProgram::unbind()
{
    glUseProgram(0);
}

ShaderProgram::Binding ShaderProgram::get_uniform_binding(UniformID id)
{
    auto iter = uniforms.find(id);
    if (iter == uniforms.end()) {
        return Binding();
    }

    return Binding((int) iter->second.first, (int) iter->second.second);
}

void ShaderProgram::uniform(const ShaderProgram::Binding& b, float v0, float v1, float v2, float v3)
{
    int id = b.first;
    if (id != -1) {
        GLenum type = (GLenum) b.second;

        switch (type) {
        case GL_FLOAT_VEC2:
            glUniform2f(id, v0, v1);
            break;
        case GL_FLOAT_VEC3:
            glUniform3f(id, v0, v1, v2);
            break;
        case GL_FLOAT_VEC4:
            glUniform4f(id, v0, v1, v2, v3);
            break;
        default:
            LOG.error("mismatch uniform binding %d", b.first);
        }
    }
}

void ShaderProgram::uniform(const ShaderProgram::Binding& b, int i0)
{
    int id = b.first;
    if (id != -1) {
        GLenum type = (GLenum) b.second;

        switch (type) {
        case GL_INT:
            glUniform1i(id, i0);
            break;
        case GL_SAMPLER_2D:
            glUniform1i(id, i0);
            break;
        case GL_SAMPLER_CUBE:
            glUniform1i(id, i0);
            break;
		case GL_BOOL:
			glUniform1i(id, i0);
			break;
        default:
            LOG.error("mismatch uniform binding (%d, %d)(u1i)", b.first, b.second);
        }
    }
}

void ShaderProgram::uniform(const ShaderProgram::Binding& b, float f0)
{
    int id = b.first;
    if (id != -1) {
        GLenum type = (GLenum) b.second;

        switch (type) {
        case GL_FLOAT:
            glUniform1f(id, f0);
            break;
        default:
            LOG.error("mismatch uniform binding %d", b.first);
        }
    }
}

void ShaderProgram::uniform(const Binding& b, GLsizei count, GLboolean transpose, const GLfloat* mat)
{
    int id = b.first;
    if (id != -1) {
        GLenum type = (GLenum) b.second;

        switch (type) {
        case GL_FLOAT_MAT4:
            glUniformMatrix4fv(id, count, transpose, mat);
            break;
        default:
            LOG.error("mismatch uniform binding %d", b.first);
        }
    }
}

