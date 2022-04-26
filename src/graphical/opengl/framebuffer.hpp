#pragma once


// local
#include "graphical/graphics.hpp"
#include "utils/asserts.hpp"

// extern
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>



class Framebuffer
{
    private:

        std::optional<GLuint> id;
        std::optional<GLuint> texture_id;
        uint64_t width;
        uint64_t height;
    
    public:

        Framebuffer(uint64_t _width, uint64_t _height): width{_width}, height{_height}
        {
            this->texture_id = 0;
            glGenTextures(1, &this->texture_id.value());
            glBindTexture(GL_TEXTURE_2D, this->texture_id.value());

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


            this->id = 0;
            glGenFramebuffers(1, &this->id.value());

            this->bind();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texture_id.value(), 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        Framebuffer(Framebuffer&& framebuffer)
        {
            leaf_assert(framebuffer.id.has_value());
            leaf_assert(framebuffer.texture_id.has_value());
            
            this->id = framebuffer.id;
            framebuffer.id = std::nullopt;

            this->texture_id = framebuffer.texture_id;
            framebuffer.texture_id = std::nullopt;

            this->width = framebuffer.width;
            this->height = framebuffer.height;
        }

        Framebuffer& operator=(Framebuffer&& framebuffer)
        {
            this->~Framebuffer();
            
            leaf_assert(framebuffer.id.has_value());
            leaf_assert(framebuffer.texture_id.has_value());
            
            this->id = framebuffer.id;
            framebuffer.id = std::nullopt;

            this->texture_id = framebuffer.texture_id;
            framebuffer.texture_id = std::nullopt;

            this->width = framebuffer.width;
            this->height = framebuffer.height;

            return *this;
        }

        ~Framebuffer()
        {
            if (this->texture_id.has_value())
                glDeleteFramebuffers(1, &this->id.value());
            
            if (this->id.has_value())
                glDeleteTextures(1, &this->texture_id.value());
        }

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;



        void bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, this->id.value());
        }

        void resize(uint64_t new_width, uint64_t new_height)
        {
            *this = Framebuffer{new_width, new_height};
        }

        void clear(const glm::u8vec4 color)
        {
            this->bind();

            glClearColor(color.r, color.g, color.b, color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glm::u64vec2 get_size()
        {
            return {this->width, this->height};
        }

        GLuint get_texture_id()
        {
            return this->texture_id.value();
        }
};