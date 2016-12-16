#include "mesh.h"
#include "log_manager.h"
#include <iostream>

using namespace std;

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, PMaterial material)
{
    this->vertices = vertices;
    this->indices = indices;
    this->material = material;
    this->setup_mesh();
}

void Mesh::draw()
{
    if (material) {
        material->get_diffuse_texture()->bind(GL_TEXTURE0);
    }

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(),GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

void Mesh::setup_mesh()
{
    glGenVertexArrays(1,&this->VAO);
    glGenBuffers(1,&this->VBO);
    glGenBuffers(1,&this->EBO);

    glBindVertexArray(this->VAO);
    glBindBuffer(GL_ARRAY_BUFFER,this->VBO);

    glBufferData(GL_ARRAY_BUFFER,this->vertices.size() * sizeof(Vertex),&this->vertices[0],GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,this->indices.size() * sizeof(GLuint),&this->indices[0],GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(GLvoid*)offsetof(Vertex,tex_coord));

    glBindVertexArray(0);
}

Model::Model(const char* path)
{
    this->load_model(path);
}

void Model::draw()
{
    for(GLuint i = 0; i < this->meshes.size(); i++){
        this->meshes[i].draw();
    }
}

void Model::load_model(std::string path){
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);

    LOG.debug("Loading model %s", path.c_str());
    if(!scene){
        LOG.error("ASSIMP::%s", import.GetErrorString());
        return ;
    }
    this->directory = path.substr(0,path.find_last_of('/'));

    this->process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode* node, const aiScene* scene){
    process_materials(scene);

    for(GLuint i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->process_mesh(mesh,scene));
    }


    for(GLuint i =0; i < node->mNumChildren; i++){
        this->process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene){
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    for(GLuint i = 0; i < mesh->mNumVertices; i++){
        Vertex vertex;
#define SET_VERTEX(n, j) vertex.position[j] = mesh->mVertices[i].n
        SET_VERTEX(x, 0);
        SET_VERTEX(y, 1);
        SET_VERTEX(z, 2);
#define SET_NORMAL(n, j) vertex.normal[j] = mesh->mNormals[i].n
        SET_NORMAL(x, 0);
        SET_NORMAL(y, 1);
        SET_NORMAL(z, 2);

        vertex.tex_coord[0] = mesh->mTextureCoords[0][i].x;
        vertex.tex_coord[1] = mesh->mTextureCoords[0][i].y;

        vertices.push_back(vertex);
    }

    for (GLuint i = 0; i < mesh->mNumFaces; i++) {
        for (GLuint j = 0; j < mesh->mFaces[i].mNumIndices; j++) {
            assert(mesh->mFaces[i].mNumIndices == 3);
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }


    PMaterial material;
    if (mesh->mMaterialIndex < materials.size()) {
        material = materials[mesh->mMaterialIndex];
    }

    return Mesh(vertices, indices, material);
}

void Model::process_materials(const aiScene* scene)
{
    materials.resize(scene->mNumMaterials);

    for (int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* a_material = scene->mMaterials[i];

        if (a_material->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
            materials[i] = nullptr;
            continue;
        }

        aiString path;
        if (a_material->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
            materials[i] = nullptr;
            continue;
        }

        string fullpath = path.data;
        int cur = fullpath.length() - 1;
        while (cur > 0 && fullpath[cur-1] != '/' && fullpath[cur-1] != '\\') cur--;
        fullpath = fullpath.substr(cur, fullpath.length() - cur);

        PMaterial material(new Material(fullpath));
        materials[i] = material;
    }
}

