
// header
#include "theme.hpp"

// extern
#include <boost/dll.hpp>



void StyleManager::load_fonts()
{
    const int MAIN_FONT_SIZE  = 14;
    const int TITLE_FONT_SIZE = 22;
    const int SUBTITLE_FONT_SIZE = 18;


    const auto program_path = boost::dll::program_location().parent_path().string();

    const auto main_font_path = program_path + "/fonts/now.regular.otf";
    const auto title_font_path = program_path + "/fonts/now.black.otf";
    const auto subtitle_font_path = program_path + "/fonts/now.medium.otf";

    this->main_font  = ImGui::GetIO().Fonts->AddFontFromFileTTF(main_font_path.c_str(), MAIN_FONT_SIZE);
    this->title_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(title_font_path.c_str(), TITLE_FONT_SIZE);
    this->subtitle_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(subtitle_font_path.c_str(), SUBTITLE_FONT_SIZE);
}


void StyleManager::apply_main_style()
{   
    auto& style    = ImGui::GetStyle();

    ImGui::StyleColorsLight();
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.13f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.71f, 0.71f, 0.71f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(1.00f, 1.00f, 1.00f, 0.53f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.40f, 0.40f, 0.40f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.65f, 0.64f, 0.64f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.79f, 0.79f, 0.79f, 0.93f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.51f, 0.51f, 0.51f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.52f, 0.52f, 0.54f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.00f, 0.00f, 0.00f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
    colors[ImGuiCol_Button]                 = ImVec4(0.81f, 0.80f, 0.80f, 0.40f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.19f, 0.19f, 0.19f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.56f, 0.57f, 0.57f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.45f, 0.45f, 0.45f, 0.95f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.79f, 0.79f, 0.80f, 0.99f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.68f, 0.69f, 0.70f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.82f, 0.82f, 0.82f, 0.70f);

    const int ROUDING_FACTOR = 12;
    style.WindowRounding = ROUDING_FACTOR;
    style.ChildRounding = ROUDING_FACTOR;
    style.TabRounding = ROUDING_FACTOR;
    style.GrabRounding = ROUDING_FACTOR;
    style.FrameRounding = ROUDING_FACTOR;
    style.PopupRounding = ROUDING_FACTOR;
    style.ScrollbarRounding = ROUDING_FACTOR;


    //colors[ImGuiCol_WindowBg] = ImVec4(1.0f,1.0f,1.0f,1.0f);
    //colors[ImGuiCol_Button]   = ImVec4(1.0f,1.0f,1.0f,1.0f);
}