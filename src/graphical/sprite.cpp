// header
#include "sprite.hpp"

// extern
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// builtin
#include <filesystem>



Sprite::Sprite(std::string const _path): path{std::move(_path)}
{
    if (!std::filesystem::exists(_path) || !std::filesystem::is_regular_file(_path))
        panic("file does not exists");

    int width;
    int height;
    uint8_t* image_texture = stbi_load(path.c_str(), &width, &height, NULL, STBI_rgb_alpha);
    this->size.x = width;
    this->size.y = height;
    
    leaf_runtime_assert(image_texture != nullptr);

    this->id = 0;
    glGenTextures(1, &this->id.value());
    glBindTexture(GL_TEXTURE_2D, this->id.value());

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (GL_UNPACK_ROW_LENGTH)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size.x, this->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image_texture);
}

Sprite::Sprite(Sprite&& sprite)
{
    std::swap(this->id, sprite.id);
    std::swap(this->size.x, sprite.size.x);
    std::swap(this->size.y, sprite.size.y);
    std::swap(this->path, sprite.path);
}

Sprite::~Sprite()
{   
    if (this->id.has_value())
        glDeleteTextures(1, &this->id.value());
}




bool SpriteManager::sprite_exists(const std::string& path)
{
    return this->sprites.find(path) != this->sprites.end();
}

void SpriteManager::load_sprite(const std::string& path)
{
    this->sprites.insert({path, Sprite{path}});
}

const Sprite& SpriteManager::get_sprite(const std::string& path)
{
    if (this->sprite_exists(path) == false)
        this->load_sprite(path);
        
    return this->sprites[path];
}

void SpriteManager::free_sprite(const std::string& path)
{
    this->sprites.erase(path);
}

void SpriteManager::clear()
{
    this->sprites.clear();
}
