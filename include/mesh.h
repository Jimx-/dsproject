#ifndef MESH_H
#define MESH_H

#include "material.h"
#include "renderable.h"
#include "intern_string.h"

#include <string>
#include <vector>
#include <map>
#include <memory>
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

    float tangent[3];

    Vertex() {
        for (int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
            bone_weights[i] = 0.0f;
        }
    }

    void add_bone_data(unsigned int bone_id, float weight) {
        for (int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
            if (bone_weights[i] == 0.0f) {
                bone_ids[i] = bone_id;
                bone_weights[i] = weight;
            }
        }
    }
};

class Mesh : public Renderable {
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

    PMaterial material;
    aiNode* scene_root;

    glm::mat4 global_transform_inverse;

    Mesh(aiNode* scene_root, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, PMaterial material,
         const BoneMapping& bones, const glm::mat4& global_transform_inverse);

    void update_bone_transform(aiAnimation* animation, float time_sec, std::vector<glm::mat4>& transforms);
    void draw(Renderer& renderer);

private:
    GLuint VAO ,VBO, EBO;
    void setup_mesh();

    void read_node_hierarchy(aiAnimation* animation, float animation_time, const aiNode* node, const glm::mat4& parent_transform);
    void interpolate_translation(aiVector3D& out, float animation_time, const aiNodeAnim* node_anim);
    void interpolate_scaling(aiVector3D& out, float animation_time, const aiNodeAnim* node_anim);
    void interpolate_rotation(aiQuaternion& out, float animation_time, const aiNodeAnim* node_anim);
	unsigned int find_rotation(float animation_time, const aiNodeAnim* node_anim);
};

class Model : public Renderable
{
public:
    Model(const char* path);

    void load_animation(InternString name, std::string path, int idx = 0);
    void draw(Renderer& renderer);

    aiAnimation* get_animation(InternString name) const;
    std::vector<Mesh>& get_meshes() { return meshes; }
private:
    std::vector<Mesh> meshes;
    std::vector<PMaterial> materials;
    std::string directory;
    std::map<InternString, aiAnimation*> animations;

    void load_model(std::string path);

    void process_node(aiNode* node, const aiScene* scene);
    Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
    void process_materials(const aiScene* scene);
};

using PModel = std::shared_ptr<Model>;

#endif

