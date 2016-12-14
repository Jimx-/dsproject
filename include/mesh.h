#ifndef MESH_H
#define MESH_H

#include "material.h"

#include <string>
#include <vector>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex{
    float position[3];
    float normal[3];
    float tex_coord[2];
};

struct Texture{
    GLuint id;
    std::string type;
};

class Mesh{
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    PMaterial material;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, PMaterial material);

    void draw();

private:
    GLuint VAO ,VBO, EBO;
    void setup_mesh();
};


class Model
{
public:
    Model(const char* path);

    void draw();
private:

    std::vector<Mesh> meshes;
    std::vector<PMaterial> materials;
    std::string directory;

    void load_model(std::string path);

    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    void process_materials(const aiScene* scene);
};

#endif

