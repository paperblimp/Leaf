#pragma once


// local
#include "graphics.hpp"
#include "utils/asserts.hpp"

// extern
#include <glm/vec2.hpp>



struct Sprite
{
    public:

        std::optional<GLuint> id;
        glm::u64vec2 size;

        std::string path;

    public:

        Sprite(const std::string _path);
        Sprite() = default;
        Sprite(Sprite&&);
        Sprite& operator=(const Sprite&) = delete;
        Sprite& operator=(Sprite&&) = delete;
        ~Sprite();

};


class SpriteManager
{
    private:

        std::unordered_map<std::string, Sprite> sprites;

    public:

        SpriteManager() = default;

        bool sprite_exists(const std::string& path);
        void load_sprite(const std::string& path);
        const Sprite& get_sprite(const std::string& path);
        void free_sprite(const std::string& path);
        void clear();

};

thread_local inline SpriteManager sprite_manager;
