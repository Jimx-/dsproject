#include "mesh.h"
#include "log_manager.h"
#include "renderer.h"
#include "exception.h"

#include <glm/gtc/type_ptr.hpp>

using namespace std;

template <typename RM, typename CM>
void copy_matrix(const RM& from, CM& to)
{
	to[0][0] = from.a1; to[1][0] = from.a2;
	to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2;
	to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2;
	to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2;
	to[2][3] = from.d3; to[3][3] = from.d4;
}

Mesh::Mesh(aiNode* scene_root, const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, PMaterial material,
           const BoneMapping& bones, const glm::mat4& global_transform_inverse)
{
    this->vertices = vertices;
    this->indices = indices;
    this->material = material;
    this->scene_root = scene_root;
    this->bones = bones;
    this->global_transform_inverse = global_transform_inverse;
    this->setup_mesh();
}

void Mesh::draw(Renderer& renderer)
{
    if (material) {
        material->get_diffuse_texture()->bind(Renderer::DIFFUSE_TEXTURE_TARGET);
        material->get_normal_map()->bind(Renderer::NORMAL_MAP_TARGET);
        renderer.uniform(ShaderProgram::MAT_METALLIC, material->get_metallic());
        renderer.uniform(ShaderProgram::MAT_ROUGHNESS, material->get_roughness());
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

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (GLvoid*)offsetof(Vertex, bone_ids));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, bone_weights));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));

    glBindVertexArray(0);
}

void Mesh::update_bone_transform(aiAnimation* animation, float time_sec, std::vector<glm::mat4>& transforms)
{
    float tick_per_sec = animation->mTicksPerSecond != 0 ?
                            animation->mTicksPerSecond : 25.0f;
    float time_tick = time_sec * tick_per_sec;
    float animation_time = fmod(time_tick, animation->mDuration);

    read_node_hierarchy(animation, animation_time, scene_root, glm::mat4());

    transforms.resize(bones.size());

    for (auto it = bones.begin(); it != bones.end(); it++) {
        int i = it->second.id;
        transforms[i] = it->second.final_transform;
    }
}

void Mesh::read_node_hierarchy(aiAnimation* animation, float animation_time, const aiNode* node, const glm::mat4& parent_transform)
{
    string node_name(node->mName.data);

    glm::mat4 node_transform;
    copy_matrix(node->mTransformation, node_transform);

    const aiNodeAnim* node_anim = nullptr;
    for (int i = 0; i < animation->mNumChannels; i++) {
        const aiNodeAnim* channel = animation->mChannels[i];
        string chan_name(channel->mNodeName.data);
        if (chan_name == node_name) node_anim = channel;
    }

    if (node_anim) {
        aiVector3D scaling_v;
        interpolate_scaling(scaling_v, animation_time, node_anim);
        aiMatrix4x4 scaling_mat;
        aiMatrix4x4::Scaling(scaling_v, scaling_mat);

        aiQuaternion rotation_q;
        interpolate_rotation(rotation_q, animation_time, node_anim);
        aiMatrix4x4 rotation_mat(rotation_q.GetMatrix());

        aiVector3D translation_v;
        interpolate_translation(translation_v, animation_time, node_anim);
        aiMatrix4x4 translation_mat;
        aiMatrix4x4::Translation(translation_v, translation_mat);

        copy_matrix(translation_mat * rotation_mat * scaling_mat, node_transform);
    }


    glm::mat4 global_transform = parent_transform * node_transform;

    if (bones.find(node_name) != bones.end()) {
        Bone& bone = bones[node_name];
        bone.final_transform = global_transform_inverse * global_transform * bone.offset_matrix;
    }

    for (unsigned int i = 0 ; i < node->mNumChildren ; i++) {
        read_node_hierarchy(animation, animation_time, node->mChildren[i], global_transform);
    }
}

void Mesh::interpolate_rotation(aiQuaternion& out, float animation_time, const aiNodeAnim* node_anim)
{
    if (node_anim->mNumRotationKeys == 1) {
        out = node_anim->mRotationKeys[0].mValue;
        return;
    }

	unsigned int rotation_index = find_rotation(animation_time, node_anim);
	unsigned int next_rotation_index = (rotation_index + 1);
    float DeltaTime = node_anim->mRotationKeys[next_rotation_index].mTime - node_anim->mRotationKeys[rotation_index].mTime;
    float Factor = (animation_time - (float)node_anim->mRotationKeys[rotation_index].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion& StartRotationQ = node_anim->mRotationKeys[rotation_index].mValue;
    const aiQuaternion& EndRotationQ = node_anim->mRotationKeys[next_rotation_index].mValue;
    aiQuaternion::Interpolate(out, StartRotationQ, EndRotationQ, Factor);
    out = out.Normalize();
}

void Mesh::interpolate_translation(aiVector3D& out, float animation_time, const aiNodeAnim* node_anim)
{
    if (node_anim->mNumPositionKeys == 1) {
        out = node_anim->mPositionKeys[0].mValue;
        return;
    }

	unsigned int position_index;
    for (position_index = 0 ; position_index < node_anim->mNumPositionKeys - 1 ; position_index++) {
        if (animation_time < (float)node_anim->mPositionKeys[position_index + 1].mTime) {
            break;
        }
    }

	unsigned int next_position_index = (position_index + 1);
    float DeltaTime = node_anim->mPositionKeys[next_position_index].mTime - node_anim->mPositionKeys[position_index].mTime;
    float Factor = (animation_time - (float)node_anim->mPositionKeys[position_index].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& StartPositionV = node_anim->mPositionKeys[position_index].mValue;
    const aiVector3D& EndPositionV = node_anim->mPositionKeys[next_position_index].mValue;
    out = EndPositionV * Factor + StartPositionV * (1 - Factor);
}

void Mesh::interpolate_scaling(aiVector3D& out, float animation_time, const aiNodeAnim* node_anim)
{
    if (node_anim->mNumScalingKeys == 1) {
        out = node_anim->mScalingKeys[0].mValue;
        return;
    }

	unsigned int scaling_index;
    for (scaling_index = 0 ; scaling_index < node_anim->mNumScalingKeys - 1 ; scaling_index++) {
        if (animation_time < (float)node_anim->mScalingKeys[scaling_index + 1].mTime) {
            break;
        }
    }

	unsigned int next_scaling_index = (scaling_index + 1);
    float DeltaTime = node_anim->mScalingKeys[next_scaling_index].mTime - node_anim->mScalingKeys[scaling_index].mTime;
    float Factor = (animation_time - (float)node_anim->mScalingKeys[scaling_index].mTime) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D& StartScalingV = node_anim->mScalingKeys[scaling_index].mValue;
    const aiVector3D& EndScalingV = node_anim->mScalingKeys[next_scaling_index].mValue;
    out = EndScalingV * Factor + StartScalingV * (1 - Factor);
}

unsigned int Mesh::find_rotation(float animation_time, const aiNodeAnim* node_anim)
{

    for (unsigned int i = 0 ; i < node_anim->mNumRotationKeys - 1 ; i++) {
        if (animation_time < (float)node_anim->mRotationKeys[i + 1].mTime) {
            return i;
        }
    }
	return 0;
}

Model::Model(const char* path)
{
    this->load_model(path);
}

void Model::draw(Renderer& renderer)
{
    for(GLuint i = 0; i < this->meshes.size(); i++){
        this->meshes[i].draw(renderer);
    }
}

void Model::load_model(std::string path) {
    Assimp::Importer* import = new Assimp::Importer();
    const aiScene* scene = import->ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);

    LOG.debug("Loading model '%s'", path.c_str());
    if(!scene){
        THROW_EXCEPT(E_RESOURCE_ERROR, "Model::load_model()", "ASSIMP::" + string(import->GetErrorString()));
    }
    this->directory = path.substr(0,path.find_last_of('/'));
    process_materials(scene);
    this->process_node(scene->mRootNode, scene);
}

void Model::load_animation(InternString name, std::string path, int idx)
{
    Assimp::Importer* import = new Assimp::Importer();
    const aiScene* scene = import->ReadFile(path,aiProcess_Triangulate | aiProcess_FlipUVs);
	scene = import->ApplyPostProcessing(aiProcess_CalcTangentSpace);

    LOG.debug("Loading animation '%s'", path.c_str());
    if(!scene){
        THROW_EXCEPT(E_RESOURCE_ERROR, "Model::load_animation()", "ASSIMP::" + string(import->GetErrorString()));
    }

    if (scene->mNumAnimations <= idx) {
        THROW_EXCEPT(E_RESOURCE_ERROR, "Model::load_animation()", "Animation '" + path + "' contains wrong number of animation nodes");
    }

    animations[name] = scene->mAnimations[idx];
}

void Model::process_node(aiNode* node, const aiScene* scene){

    for(GLuint i = 0; i < node->mNumMeshes; i++){
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		if (strcmp(mesh->mName.data, "Spiketrap") == 0) continue;	/* XXX: workaround */
        this->meshes.push_back(this->process_mesh(mesh, scene));
    }


    for(GLuint i =0; i < node->mNumChildren; i++){
        this->process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene){
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;


	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
#define SET_VERTEX(n, j) vertex.position[j] = mesh->mVertices[i].n
		SET_VERTEX(x, 0);
		SET_VERTEX(y, 1);
		SET_VERTEX(z, 2);
#define SET_NORMAL(n, j) vertex.normal[j] = mesh->mNormals[i].n
		SET_NORMAL(x, 0);
		SET_NORMAL(y, 1);
		SET_NORMAL(z, 2);
#define SET_TANGENT(n, j) vertex.tangent[j] = mesh->mTangents[i].n
		SET_TANGENT(x, 0);
		SET_TANGENT(y, 1);
		SET_TANGENT(z, 2);

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

    Mesh::BoneMapping bones;
    int num_bones = 0;

	/* default bone data */
	if (!mesh->mNumBones) {
		for (GLuint i = 0; i < mesh->mNumVertices; i++) {
			vertices[i].add_bone_data(1, 1.0f);
		}
	} else {
		for (unsigned int i = 0; i < mesh->mNumBones; i++) {
			string bone_name(mesh->mBones[i]->mName.data);

			int bone_idx;
			if (bones.find(bone_name) == bones.end()) {
				bone_idx = num_bones++;

				Mesh::Bone new_bone;
				new_bone.id = bone_idx;
				bones[bone_name] = new_bone;
			}
			else {
				bone_idx = bones[bone_name].id;
			}

			bones[bone_name].id = bone_idx;
			copy_matrix(mesh->mBones[i]->mOffsetMatrix, bones[bone_name].offset_matrix);

			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
				unsigned int vid = mesh->mBones[i]->mWeights[j].mVertexId;
				float weight = mesh->mBones[i]->mWeights[j].mWeight;
				vertices[vid].add_bone_data(bone_idx, weight);
			}
		}
	}

    glm::mat4 global_transform;
    copy_matrix(scene->mRootNode->mTransformation, global_transform);
    return Mesh(scene->mRootNode, vertices, indices, material, bones, glm::inverse(global_transform));
}

void Model::process_materials(const aiScene* scene)
{
    materials.resize(scene->mNumMaterials);

    for (int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* a_material = scene->mMaterials[i];

        float roughness = 0.8f;
        float metallic = 0.0f;

		string normal_map = "flat_normal_map.png";
		string diffuse_map = "";

        /* XXX: workaround: the model assets have quite broken material info, so specify them here manually */
        aiString mat_name;
        if (AI_SUCCESS == a_material->Get(AI_MATKEY_NAME, mat_name)) {
			string mat_str(mat_name.data);
            if (mat_str.find("equipment", 0) != string::npos) {	/* skeleton's equipment */
                metallic = 0.8f;
                roughness = 0.5f;
			} else if (mat_str.find("Spike", 0) != string::npos) { /* trap's spike */
				metallic = 0.8f;
				roughness = 0.3f;
				normal_map = "Spiketrap_normal_map.tga";
			} else if (mat_str.find("Body", 0) != string::npos) { /* trap's body */
				metallic = 0.3f;
				roughness = 0.7f;
				normal_map = "Spiketrap_normal_map.tga";
			} else if (mat_str == "lambert6") {	/* treasure */
				metallic = 0.8f;
				roughness = 0.3f;
			} else if (mat_str == "Barrel1") {
				diffuse_map = "Barrel2_A.png";
				normal_map = "Barrel2_N.png";
			}
        }

		if (diffuse_map == "") {
			if (a_material->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
				materials[i] = nullptr;
				continue;
			}

			aiString path;
			if (a_material->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
				materials[i] = nullptr;
				continue;
			}

			diffuse_map = path.data;
			int cur = diffuse_map.length() - 1;
			while (cur > 0 && diffuse_map[cur - 1] != '/' && diffuse_map[cur - 1] != '\\') cur--;
			diffuse_map = diffuse_map.substr(cur, diffuse_map.length() - cur);
		}

		aiString path;
        if (a_material->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
			normal_map = path.data;
			int cur = normal_map.length() - 1;
			while (cur > 0 && normal_map[cur-1] != '/' && normal_map[cur-1] != '\\') cur--;
			normal_map = normal_map.substr(cur, normal_map.length() - cur);
        }

		PMaterial material(new Material(roughness, metallic, diffuse_map, normal_map));
        materials[i] = material;
    }
}

aiAnimation* Model::get_animation(InternString name) const
{
    auto it = animations.find(name);
    if (it == animations.end()) {
        return nullptr;
    }
    return it->second;
}


