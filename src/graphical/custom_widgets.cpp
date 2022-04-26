#include "custom_widgets.hpp"

void CustomImGui::Title(const char* text)
{   
    ImGui::PushFont(style_manager.title_font);
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
}

void CustomImGui::SubTitle(const char* text)
{   
    ImGui::PushFont(style_manager.subtitle_font);
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
}