#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <memory>
#include <glad/glad.h>

class MaterialTexture;
using PMaterialTexture = std::shared_ptr<MaterialTexture>;

class MaterialTexture {
public:
    MaterialTexture(const std::string& path);

    void load_texture();
    void bind(GLuint target);

    static PMaterialTexture create_texture(const std::string& name);
private:
    bool loaded;
    std::string path;
    GLuint handle;
};

class Material {
public:
    Material(float roughness, float metallic, const std::string& diffuse_texture);

    PMaterialTexture get_diffuse_texture() const { return diffuse_texture; }
    float get_metallic() const { return metallic; }
    float get_roughness() const { return roughness; }
private:
    PMaterialTexture diffuse_texture;
    float roughness;
    float metallic;
};

using PMaterial = std::shared_ptr<Material>;

#endif
