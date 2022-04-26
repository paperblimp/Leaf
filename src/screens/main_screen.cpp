// header
#include "screens/main_screen.hpp"

// builtin
#include <GLFW/glfw3.h>
#include <chrono>
#include <imgui.h>
#include <thread>

// local
#include "screens/projects_screen.hpp"
#include "sections/properties_and_preview.hpp"
#include "sections/filesystem.hpp"
#include "sections/keyframe_widget.hpp"
#include "sections/node_renderer.hpp"
#include "sections/viewport.hpp"
#include "sections/main_bar.hpp"

#include "dialogs/file_browser.hpp"
#include "dialogs/choise.hpp"

#include "graphical/graphics.hpp"
#include "graphical/sprite.hpp"
#include "graphical/opengl/framebuffer.hpp"
#include "graphical/opengl/render.hpp"
#include "graphical/sprite.hpp"

#include "sections/viewport.hpp"
#include "sections/depth_indicator.hpp"
#include "sections/action_bar.hpp"
#include "sections/section.hpp"

#include "config.hpp"
#include "node_tree.hpp"
#include "utils/serialization.hpp"
#include "history.hpp"
#include "graphical/theme.hpp"
#include "key_map.hpp"



struct KeyEvent
{
    GLFWwindow* window;
    int key;
    int scancode;
    int action;
    int mods;
};

std::optional<KeyEvent> key_event;
TextInput new_node_name_input = TextInput{"new node", "new node name"};
TextInput rename_name_input = TextInput{"rename node", "new name"};


static std::function<void(GLFWwindow*, int, int, int)> mouse_input_callback = [](...){ panic("uninitialized callback function"); };
void __glfw_mouse_input_callback(GLFWwindow* window, int button, int action, int mods)
{
    mouse_input_callback(window, button, action, mods);
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}


void keyboard_input_handler(KeyEvent key_event, Viewport& viewport)
{
        if (!(key_event.mods & GLFW_MOD_CONTROL))
            return;
            
        if (key_event.action == GLFW_REPEAT || key_event.action == GLFW_RELEASE)
            return;

        if (key_event.key == GLFW_KEY_Z)
        {
            history->undo();
            viewport.stop_current_action();
        }
        
        else if (key_event.key == GLFW_KEY_Y)
        {
            history->redo();
            viewport.stop_current_action();
        }


        if (node_tree->selected_nodes.empty() == false) // && ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel) == false)
        {
            if (node_tree->selected_nodes.size() == 1)
            {    
                // add node / ctrl-a

                if (key_event.key == GLFW_KEY_A)
                    new_node_name_input.open("");


                // rename node / ctrl-r

                if (key_event.key == GLFW_KEY_R)
                    rename_name_input.open("");


                // duplicate node / ctrl-d

                if (key_event.key == GLFW_KEY_D)
                {
                    auto node = node_tree->selected_nodes[0];
                    node->get_parent()->duplicate_child(node->get_idx());
                }
            

                // move down / ctrl-down

                if (key_event.key == GLFW_KEY_DOWN)
                {
                    auto node = node_tree->selected_nodes[0];

                    if (node->get_idx() < (node->get_parent()->get_child_count() - 1))
                        node->get_parent()->reorder_child(node->get_idx(), node->get_idx() + 1);
                }
            

                // move up / ctrl-up

                if (key_event.key == GLFW_KEY_UP)
                {
                    auto node = node_tree->selected_nodes[0];

                    if (node->get_idx() != 0)
                        node->get_parent()->reorder_child(node->get_idx(), node->get_idx() - 1);
                }
            }

            // delete / node del

            if (key_event.key == GLFW_KEY_DELETE)
            {   
                std::vector<std::unique_ptr<Action>> actions;

                for (auto node: node_tree->selected_nodes)
                {
                    if (node->is_rootless())
                        continue;

                    node->get_parent()->remove_children(node->get_idx(), false);
                    actions.push_back(std::make_unique<RemoveNode>(node, node->get_parent()));
                }

                history->push_action(std::make_unique<ActionGroup>(std::move(actions)));
            }
        }
}

void input_handler_popups()
{
    // add node / ctrl-a

    auto new_node_name = new_node_name_input.run();
    if (new_node_name.has_value())
        node_tree->selected_nodes[0]->add_child(new_node_name.value());


    // rename node / ctrl-r

    auto rename_name = rename_name_input.run();
    if (rename_name.has_value())
        node_tree->selected_nodes[0]->rename(rename_name.value());
}


std::tuple<bool, void*, nlohmann::json> main_screen(nlohmann::json)
{

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    auto return_value = std::tuple<bool, void*, nlohmann::json>{false, nullptr, nlohmann::json::object()};
    bool should_stop = false;


    MainBar main_bar{};


    Viewport viewport;
    DepthIndicator depth_indicator;
    NodeRenderer node_renderer{};
    KeyframeWidget keyframe_widget{};
    Filesystem  filesystem{std::filesystem::current_path().string()};
    PropertiesAndPreview properties_and_preview{};


    //Initialize the action bar with SectionButtons for all sections that can be interacted on the action_bar
    ActionBar action_bar
    {

        std::unordered_map<ButtonName,SectionButton>
        {
            {ASSETS, SectionButton{
                    ASSETS,"assets",
                    std::unordered_map<Slot,Section*>{
                        {LEFT_SLOT,&filesystem}
                    }
                } 
            },
            {LAYERS,SectionButton{
                    LAYERS,"layers",
                    std::unordered_map<Slot,Section*>{
                        {RIGHT_SLOT,&node_renderer}
                    }
                }
            },
            {ANIMATION_CONTROL,SectionButton{
                    ANIMATION_CONTROL,"properties",
                    std::unordered_map<Slot,Section*>{
                        {LEFT_SLOT,&properties_and_preview},
                        {CENTER_SLOT,&keyframe_widget}
                    }
                }
            },
        }
    };

    Choise save_on_quit{"Exit","Would you like to save before exiting?\n",
        std::vector<std::string> {
            "Save & Exit",
            "Cancel",
            "Don't Save"
        }
    };


    // window size

    glfwSetWindowSize(graphic_context.window, config.current_project.header.window_size.y, config.current_project.header.window_size.x);

    glfwSetWindowSizeCallback(graphic_context.window, [](GLFWwindow*, int height, int width)
    {
        config.current_project.header.window_size.x = height;
        config.current_project.header.window_size.y = width;
    });

    // input setting

    struct MouseEvent
    {
        GLFWwindow* window;
        int button;
        int action;
        int mods;
    };

    std::queue<MouseEvent> mouse_event_queue;

    glfwSetMouseButtonCallback(graphic_context.window, __glfw_mouse_input_callback);
    mouse_input_callback = [&mouse_event_queue](GLFWwindow* window, int button, int action, int mods)
    {
        mouse_event_queue.push(MouseEvent{window, button, action, mods});
    };


    glfwSetKeyCallback(graphic_context.window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
    
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        
        key_event = {window, key, scancode, action, mods};

        if (action == GLFW_PRESS)
            key_map[key] = true;

        if (action == GLFW_RELEASE)
            key_map[key] = false;
    });


    // main loop
    while (!should_stop)
    {
        // input
        glfwPollEvents();

        if (key_event.has_value() && key_event.value().key == GLFW_KEY_LEFT_CONTROL && key_event.value().action == GLFW_PRESS)
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        
        if (key_event.has_value() && key_event.value().key == GLFW_KEY_LEFT_CONTROL && key_event.value().action == GLFW_RELEASE)
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


        graphic_context.start_frame();
        

        // mouse events
        while (mouse_event_queue.empty() == false)
        {
            auto mouse_event = mouse_event_queue.front();
            mouse_event_queue.pop();

            viewport.process_input(mouse_event.button, mouse_event.action);
        }

        // keyboard events
        if (key_event.has_value())
            keyboard_input_handler(key_event.value(), viewport);

        key_event = std::nullopt;
        input_handler_popups();


        // create dockspace
        ImGui::DockSpaceOverViewport();

        //Update anim time
        anim_data.update(ImGui::GetIO().DeltaTime);

        //Render windows
        action_bar.render();
        viewport.render();
        depth_indicator.render();

        //Render sections
        filesystem.render_section();
        node_renderer.render_section();
        keyframe_widget.render_section();
        properties_and_preview.render_section();


        if(main_bar.render())
        {
            return_value = {true, (void*)projects_screen, nlohmann::json::object()};
            should_stop = true;
        }

        ImGui::ShowDemoWindow();


        //Ask for save before quiting 
        if(glfwWindowShouldClose(graphic_context.window))
        {
            save_on_quit.open();
            glfwSetWindowShouldClose(graphic_context.window, false);
        }

        if(auto quit_input = save_on_quit.run(); quit_input.has_value())
        {
            if(quit_input.value() == "Save & Exit") 
            {
                serialize_project(config.current_project.header.path);
                should_stop = true;
            }
            else if(quit_input.value() == "Don't Save")
            {
                should_stop = true;
            }
        }

        graphic_context.end_frame(clear_color);
        graphic_context.display_frame();
    }

    return return_value;
}