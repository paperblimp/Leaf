#include "sections/section.hpp"
#include "sections/property_editor.hpp"
#include "sections/preview.hpp"

#include "graphical/custom_widgets.hpp"

#include <imgui.h>

class PropertiesAndPreview : public Section
{
    PropertyEditor property_editor{};
    Preview preview{};

    const uint8_t SUBSECTION_OFFSET_Y = 2;

    void render()
    {
        //Directly render property editor and preview as child windows without render_section() checkage
        ImGui::Begin("##Animation Controls",NULL,SECTION_FLAGS);
        CustomImGui::Title("Animation Controls");

        const ImVec2 content_size = ImGui::GetContentRegionAvail();
        const ImVec2 sub_windows_size
        {
            content_size.x,
            content_size.y/2 - SUBSECTION_OFFSET_Y
        };

        //ImGui::SetNextWindowSize(sub_windows_sizes);
        property_editor.render(sub_windows_size);

        //ImGui::SetNextWindowSize(sub_windows_sizes);
        preview.render_subsection(sub_windows_size);

        ImGui::End();
    }

};