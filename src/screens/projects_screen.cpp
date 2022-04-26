// header
#include "screens/projects_screen.hpp"

// local
#include "config.hpp"
#include "graphical/graphics.hpp"
#include "screens/main_screen.hpp"
#include "dialogs/file_browser.hpp"
#include "dialogs/preferences.hpp"
#include "utils/serialization.hpp"

// builtin
#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fmt/core.h>
#include <imgui.h>
#include <optional>
#include <unordered_set>
#include <functional>
#include <string.h>

// extern
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



Alert no_name_provided   {"No Name Provided","Please choose a name for your project"};
Alert no_path_provided   {"No Path Provided","Please choose a path for your project"};
Alert name_already_in_use{
    "Name already in use",
    "There's already a project with the given name on the choosen folder.\nPlease choose another one"};


std::optional<std::string> leaf_project_file_filter(std::filesystem::path path)
{
    if (path.extension() != ".leafproject")
        return "Invalid extension, please import a .leafproject file.";
    else
        return std::nullopt;
};

FileBrowser new_project_browser    {"New project",FileBrowser::Type::Folder};
FileBrowser import_project_browser {"Import project",FileBrowser::Type::File,&leaf_project_file_filter};

PreferencesDialog preferences_dialog{};




std::tuple<bool, void*, nlohmann::json> projects_screen(nlohmann::json)
{
   
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    auto return_value = std::tuple<bool, void*, nlohmann::json>{false, nullptr, nlohmann::json::object()};
    bool should_stop = false;

    // remove inexistent projects
    auto new_end = std::remove_if(config.projects.begin(), config.projects.end(), [](auto& project)
    {
        return !std::filesystem::exists(project.path);
    });

    config.projects.erase(new_end, config.projects.end());

    

    while (!glfwWindowShouldClose(graphic_context.window) && !should_stop)
    {
        // input
        glfwPollEvents();

        graphic_context.start_frame();


        // create dockspace
        ImGui::DockSpaceOverViewport();
    
        ImGui::Begin("Projects", nullptr, ImGuiWindowFlags_AlwaysAutoResize);


        std::unordered_set<std::string> valid_orderings{"name", "last access date"};

        const std::string default_ordering = "name";
        assert(valid_orderings.find(default_ordering) != valid_orderings.end());

        static std::string current_ordering = default_ordering;
        if (ImGui::BeginCombo("ordering", current_ordering.c_str()))
        {
            for (auto& ordering: valid_orderings)
                if (ImGui::Selectable(ordering.c_str(), current_ordering == ordering))
                    current_ordering = ordering;

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        static std::array<char, 6666> name_filter{};
        ImGui::InputText("filter", name_filter.data(), name_filter.size());



        std::vector<ApplicationConfig::Project::Header*> projects;
        projects.reserve(config.projects.size());
        
        // filter and insert projects
        for (auto& project: config.projects)
            if (std::string_view{name_filter.data()}.empty() || project.name.find(name_filter.data()) != project.name.npos)
                projects.push_back(&project);

        // sort projects
        if (current_ordering == "name")
            std::sort(projects.begin(), projects.end(), [](auto n1, auto n2){ return n1->name < n2->name; });
        else if (current_ordering == "last access date")
            std::sort(projects.begin(), projects.end(), [](auto n1, auto n2){ return n1->last_access < n2->last_access; });
        else
            panic("fatal error");
        
        
        // render table
        auto flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
        if (ImGui::BeginTable("projects", 4, flags, {0, ImGui::GetTextLineHeight() * 35}))
        {
            // headers
            ImGui::TableSetupColumn("name");
            ImGui::TableSetupColumn("path");
            ImGui::TableSetupColumn("last access date");
            ImGui::TableSetupColumn("");
            ImGui::TableHeadersRow();

            // table items
            for (auto project: projects)
            {
                ImGui::PushID(project->path.c_str());
                ImGui::PushID(boost::posix_time::to_simple_string(project->last_access).c_str());

                //Row click
                ImGui::TableNextRow();

                // name
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(project->name.c_str());
                
                // path
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(project->path.string().c_str());

                // date
                ImGui::TableSetColumnIndex(2);
                auto time = boost::posix_time::to_simple_string(project->last_access);
                ImGui::TextUnformatted(time.c_str());


                // delete
                ImGui::TableSetColumnIndex(3);
                if (ImGui::SmallButton("forget"))
                {
                    for (auto it = config.projects.begin(); it != config.projects.end(); ++it)
                    {
                        if (it->name == project->name)
                        {
                            config.projects.erase(it);
                            break;
                        }
                    }
                }

                // Open project input
                ImGui::TableSetColumnIndex(0);
                bool row_clicked = false;
                ImGui::Selectable("##Selectable",&row_clicked,ImGuiSelectableFlags_SpanAllColumns);

                if(row_clicked)
                {
                    load_project(project->path);
                    return_value = {true, (void*)main_screen, nlohmann::json::object()};
                    should_stop = true;
                }

                ImGui::PopID();
                ImGui::PopID();
            }

            ImGui::EndTable();
        }



        if (ImGui::Button("Create Project")) ImGui::OpenPopup("Create Project");
        //True when user creates a project
        if(render_create_popup())
        {
            return_value = {true, (void*)main_screen, nlohmann::json::object()};
            should_stop = true;
        }


        ImGui::SameLine();

        if (ImGui::Button("Import Project"))
        {
            auto state = FileBrowser::State::get_default();
            state.current_path = std::filesystem::current_path();
            import_project_browser.open(state);
        }

        auto import_path = import_project_browser.run();
        if(import_path.has_value())  import_project(import_path.value());

        ImGui::End();

        graphic_context.end_frame(clear_color);
        graphic_context.display_frame();

    }

    return return_value; 
}


bool render_create_popup()
{
    bool return_value = false;

    static std::array<char, 666> name_buffer;
    static std::string           project_folder;
    static std::string           project_name;

    if(project_folder.empty()) project_folder = std::filesystem::current_path().string();


    if(ImGui::BeginPopupModal("Create Project",nullptr,ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Name  ");ImGui::SameLine();
        if(ImGui::InputText("##ProjectName", (char*)name_buffer.data(), name_buffer.size()))
        {
            project_name = name_buffer.data();
        }

        ImGui::Text("Folder");ImGui::SameLine(); 
        if(ImGui::SameLine();ImGui::Button("..."))
        {
            auto state = FileBrowser::State::get_default();
            state.current_path = std::filesystem::current_path();
            new_project_browser.open(state); 
        }
        auto path = new_project_browser.run();
        if(path.has_value())
        {
            project_folder = path.value();
        }

        
        if(ImGui::Button("Preferences")) preferences_dialog.open();
        preferences_dialog.run();

        if(ImGui::NewLine();ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();

        if(ImGui::SameLine();ImGui::Button("Create"))
        {
            std::filesystem::path final_path = std::filesystem::path{project_folder} / std::filesystem::path{fmt::format("{}.leafproject", project_name)};
            if(project_name.empty())                     no_name_provided.open();
            else if(project_folder.empty())              no_path_provided.open();
            else if(std::filesystem::exists(final_path)) name_already_in_use.open();
            else
            {
                create_project(project_name,project_folder);
                name_buffer = std::array<char, 666>{};
                project_folder = std::string{};
                project_name   = std::string{};
                return_value = true;
            }
        }

        no_name_provided.run();
        no_path_provided.run();
        name_already_in_use.run();

        /* 
        auto state = FileBrowser::State::get_default();
        state.current_path = std::filesystem::current_path();
        file_browser.open(state);*/

        ImGui::EndPopup();
    }

    return return_value;

    
}

void create_project(const std::string& name,const std::string& folder)
{
    std::filesystem::path folder_path{folder};
    std::filesystem::path file_path  {fmt::format("{}.leafproject", name)};
    std::filesystem::path final_path {folder_path / file_path};

    int width;
    int height;
    glfwGetWindowSize(graphic_context.window, &width, &height);


    config.current_project.header = ApplicationConfig::Project::Header{name,final_path,boost::posix_time::second_clock::local_time(), {height, width}};
    config.projects.push_back(config.current_project.header);

    serialize_project(final_path);

}

void import_project(const std::string& path)
{
    load_project(path);
    config.projects.push_back(config.current_project.header);
}