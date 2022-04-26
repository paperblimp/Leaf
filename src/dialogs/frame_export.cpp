
#define STB_IMAGE_WRITE_IMPLEMENTATION

// header
#include "frame_export.hpp"

// local
#include "node_tree.hpp"
#include "graphical/sprite.hpp"
#include "graphical/opengl/framebuffer.hpp"
#include "graphical/opengl/render.hpp"

// extern
#include <glm/vec2.hpp>
#include <stb_image_write.h>



void FrameExportDialog::open()
{
    ImGui::OpenPopup(FrameExportDialog::dialog_name.c_str());
}

std::optional<std::string> FrameExportDialog::browser_filter(const std::filesystem::path path)
{
    if (path.extension() != ".png")
        return "only \".png\" is supported";
    else
        return std::nullopt;
}

void FrameExportDialog::run()
{
    if (ImGui::BeginPopupModal(FrameExportDialog::dialog_name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize) == false)
        return;

    // ┌─────────────────────┐             ┌───────────┐
    // │                     │ output path │select path│
    // └─────────────────────┘             └───────────┘
    // 
    // ┌──────┐ ┌──────┐
    // │cancel│ │export│
    // └──────┘ └──────┘

    ImGui::InputText("output path", this->output_path.data(), this->output_path.size());
    ImGui::SameLine();
    if (ImGui::Button("select path"))
        this->file_browser.open(FileBrowser::State::default_with_current_path());


    if (ImGui::Button("cancel"))
        ImGui::CloseCurrentPopup();
    
    ImGui::SameLine();

    if (ImGui::Button("export"))
    {
        this->export_frame(this->output_path.data());
        ImGui::CloseCurrentPopup();
    }

    if (auto result = this->file_browser.run(); result.ended() && result.has_value())
        strcpy(this->output_path.data(), result.value().c_str());

    ImGui::EndPopup();
}


void render_current_frame(Framebuffer& framebuffer)
{
    framebuffer.clear({255, 255, 255, 255});

    node_tree->run_on_nodes_ordered_reverse([&](Node& node)
    {
        if (node.texture_path.has_value() == false)
            return;

        if (node.visible == false)
            return;

        auto& sprite = sprite_manager.get_sprite(node.texture_path.value());
        render_sprite(
                sprite,
                node.position,
                (glm::dvec2)sprite.size * (glm::dvec2)node.scale,
                glm::degrees(node.rotation),
                {node.rotation_pivot[0] + node.position.x, node.rotation_pivot[1] + node.position.y},
                framebuffer
        );
    });
}

void FrameExportDialog::export_frame(const std::string& path)
{

    glm::u64vec2 frame_size = get_camera_area();
    auto framebuffer = Framebuffer{frame_size.x, frame_size.y};

    render_current_frame(framebuffer);

    const size_t ARGB_SIZE = 4;
    auto pixels = std::vector<uint8_t>(frame_size.x * frame_size.y * ARGB_SIZE, 0);
    
    framebuffer.bind();
    glReadPixels(0, 0, frame_size.x, frame_size.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    auto err = stbi_write_png(path.c_str(), frame_size.x, frame_size.y, 4, pixels.data(), 0);
    leaf_assert(err != 0);
}
