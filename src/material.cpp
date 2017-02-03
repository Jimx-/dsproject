#include "material.h"
#include "log_manager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb.h>
#include <stb_image.h>
#include <map>

using namespace std;

static map<string, PMaterialTexture> texture_cache = {};

static const string TEXTURE_DIR = "resources/textures/";

MaterialTexture::MaterialTexture(const std::string& path)
{
    handle = 0;
    this->path = path;
}

void MaterialTexture::load_texture()
{
    LOG.debug("Loading texture '%s'", path.c_str());

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
    // Set our texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load, create texture and generate mipmaps
    int width, height;
    unsigned char* image = stbi_load(path.c_str(), &width, &height, 0, STBI_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    handle = texture;
}

void MaterialTexture::bind(GLuint target)
{
    glActiveTexture(target);
    glBindTexture(GL_TEXTURE_2D, handle);
}

PMaterialTexture MaterialTexture::create_texture(const std::string& name)
{
    auto it = texture_cache.find(name);
    if (it == texture_cache.end()) {
        PMaterialTexture texture(new MaterialTexture(TEXTURE_DIR + name));
        texture->load_texture();
        texture_cache[name] = texture;
        return texture;
    }
    return it->second;
}

Material::Material(float roughness, float metallic, const std::string& diffuse_texture, const std::string& normal_map)
{
    this->roughness = roughness;
    this->metallic = metallic;
    this->diffuse_texture = MaterialTexture::create_texture(diffuse_texture);
    this->normal_map = MaterialTexture::create_texture(normal_map);
}

