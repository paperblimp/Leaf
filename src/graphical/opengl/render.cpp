// header
#include "render.hpp"



void render(std::function<void(void)> func, double angle, glm::vec2 pivot, Framebuffer& framebuffer);



void render_color(const glm::u8vec4 color, const glm::vec2 position, glm::vec2 size, double angle, Framebuffer& framebuffer)
{
    render_color(color, position, size, angle, glm::vec2{position + (size / glm::vec2{2, 2})}, framebuffer);
}

void render_color(const glm::u8vec4 color, const glm::vec2 position, glm::vec2 size, double angle, glm::vec2 pivot, Framebuffer& framebuffer)
{
    
    const auto func = [color, position, size]()
    {
        const glm::vec2 absolute_position = position - (glm::vec2{size.x, size.y} / glm::vec2{2, 2});


        glBegin(GL_QUADS);
        glColor4f((float)color.r / 255, (float)color.g / 255, (float)color.b / 255, (float)color.a / 255);

        glVertex2f(absolute_position.x, absolute_position.y);
        glVertex2f(absolute_position.x + size.x, absolute_position.y);
        glVertex2f(absolute_position.x + size.x, absolute_position.y + size.y);
        glVertex2f(absolute_position.x, absolute_position.y + size.y);
        
        glColor4f(1, 1, 1, 1);
        glEnd();
    };

    render(func, angle, pivot, framebuffer);
}

void render_sprite(const Sprite& sprite, glm::vec2 position, glm::vec2 size, double angle, Framebuffer& framebuffer)
{
    render_sprite(sprite, position, size, angle, position, framebuffer);
}

void render_sprite(const Sprite& sprite, glm::vec2 position, const glm::vec2 size, double angle, glm::vec2 pivot, Framebuffer& framebuffer)
{
    const auto func = [&sprite, position, size]()
    {
        // carrega a textura
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, sprite.id.value());

        glBegin(GL_QUADS);

            const glm::vec2 absolute_position = position - (glm::vec2{size.x, size.y} / glm::vec2{2, 2});

            // DEBUG:
            // notice(fmt::format("viewport size: {} {}", width, height));
            // notice(fmt::format("center position: {} {}", position.x, position.y));
            // notice(fmt::format("absolute position: {} {}", absolute_position.x, absolute_position.y));
            // notice(fmt::format("size: {} {}", size.x, size.y));

            glTexCoord2f(0.f, 0.f); glVertex2f(absolute_position.x, absolute_position.y);
            glTexCoord2f(1.f, 0.f); glVertex2f(absolute_position.x + size.x, absolute_position.y);
            glTexCoord2f(1.f, 1.f); glVertex2f(absolute_position.x + size.x, absolute_position.y + size.y);
            glTexCoord2f(0.f, 1.f); glVertex2f(absolute_position.x, absolute_position.y + size.y);

        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
    };

    render(func, angle, pivot, framebuffer);
}

void render(std::function<void(void)> func, double angle, glm::vec2 pivot, Framebuffer& framebuffer)
{
    auto framebuffer_size = framebuffer.get_size();


    framebuffer.bind();

    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, framebuffer_size.x, framebuffer_size.y);

    // impede que a imagem seja rendenizada invertida
    glScalef(1, -1, 1);

    // modifica as dimensões do clip space
    glOrtho(0, framebuffer_size.x, framebuffer_size.y, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // rotaciona os vértices
    glTranslatef(pivot.x, pivot.y, 0);
    glRotated(angle, 0, 0, 1);
    glTranslatef(-pivot.x, -pivot.y, 0);


    func();


    // unbind do framebuffer atual
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
