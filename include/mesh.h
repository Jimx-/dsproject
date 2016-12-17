#ifndef MESH_H
#define MESH_H

#include "material.h"
#include "intern_string.h"

#include <string>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

#define NUM_BONES_PER_VERTEX    4
struct Vertex{
    float position[3];
    float normal[3];
    float tex_coord[2];

    int bone_ids[NUM_BONES_PER_VERTEX];
    float bone_weights[NUM_BONES_PER_VERTEX];

    Vertex() {
        for (int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
            bone_weights[i] = 0.0f;
        }
    }

    void add_bone_data(uint bone_id, float weight) {
        for (int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
            if (bone_weights[i] == 0.0f) {
                bone_ids[i] = bone_id;
                bone_weights[i] = weight;
            }
        }
    }
};

struct Texture{
    GLuint id;
    std::string type;
};

class Mesh {
public:

    struct Bone {
        int id;
        glm::mat4 offset_matrix;
        glm::mat4 final_transform;
    };

    using BoneMapping = std::map<std::string, Bone>;

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    BoneMapping bones;

    aiNode* scene_root;
    aiAnimation* animation;
    float animation_time_sec;

    PMaterial material;

    Mesh(aiNode* scene_root, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, PMaterial material, const BoneMapping& bones);
    //Mesh(const Mesh& that);
    //Mesh(Mesh&& that);

    void start_animation(aiAnimation* animation);
    void update_animation(float dt);

    void draw();


private:
    GLuint VAO ,VBO, EBO;
    void setup_mesh();

    void update_bone_transform(float time_sec, std::vector<glm::mat4>& transforms);
    void read_node_hierarchy(float animation_time, const aiNode* node, const glm::mat4& parent_transform);
    void interpolate_rotation(aiQuaternion& out, float animation_time, const aiNodeAnim* node_anim);
    uint find_rotation(float animation_time, const aiNodeAnim* node_anim);
};

class Model
{
public:
    Model(const char* path);

    void load_animation(InternString name, std::string path);
    void draw();

    void start_animation(InternString name);
    void update_animation(float dt);
private:
    std::vector<Mesh> meshes;
    std::vector<PMaterial> materials;
    std::string directory;
    std::map<InternString, aiAnimation*> animations;
    aiAnimation* current_animation;

    void load_model(std::string path);

    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    void process_materials(const aiScene* scene);
};

#endif

