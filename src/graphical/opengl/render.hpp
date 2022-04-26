#pragma once


// local
#include "graphical/graphics.hpp"
#include "graphical/sprite.hpp"
#include "framebuffer.hpp"

// extern
#include <glm/vec2.hpp>



void render_sprite(const Sprite& sprite, glm::vec2 position, glm::vec2 size, double angle, Framebuffer& framebuffer);
void render_sprite(const Sprite& sprite, glm::vec2 position, glm::vec2 size, double angle, glm::vec2 pivot, Framebuffer& framebuffer);
void render_color(const glm::u8vec4 color, const glm::vec2 position, glm::vec2 size, double angle, Framebuffer& framebuffer);
void render_color(const glm::u8vec4 color, const glm::vec2 position, glm::vec2 size, double angle, glm::vec2 pivot, Framebuffer& framebuffer);
