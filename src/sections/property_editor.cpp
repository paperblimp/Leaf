
// header
#include "property_editor.hpp"

// local
#include "animation/easings.hpp"
#include "animation/keyframe.hpp"
#include "sections/keyframe_widget.hpp"
#include "node_tree.hpp"
#include "utils/math_utils.hpp"
#include "sections/node_history.hpp"
#include "animation/animation.hpp"
#include "graphical/custom_widgets.hpp"

// builtin
#include <optional>
#include <cstring>

// extern
#include <glm/ext/vector_float2.hpp>
#include <imgui.h>


void PropertyEditor::render(const ImVec2& size)
{   //Render the PropertyEditor and update the values of the selected_node
    
    ImGui::BeginChild("Property Editor",size,true);
    CustomImGui::SubTitle("Property Editor");

    if(node_tree->selected_nodes.empty())
    {
        ImGui::Text("Nothing selected");
        ImGui::EndChild();
        return;
    }

    if (this->current_node.has_value() == false || this->current_node.value() != node_tree->selected_nodes[0])
    {
        this->current_node = node_tree->selected_nodes[0];
        this->reset_status();
        this->init_status(*this->current_node.value());
    }

    auto& node = this->current_node.value();

    //General properties Tree
    if(ImGui::TreeNode("General Properties"))
    {
        if(ImGui::BeginTable("General Properties", 2,
        ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
        {
            ImGui::TableNextColumn();

            ImGui::Text("Name");
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(node->name.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("Visible");
            ImGui::TableNextColumn();
            
            if (ImGui::Checkbox("##PropertyVisible", &(node->visible)))
                history->push_action(std::make_unique<NodeMemberEdit<bool>>(node, node->visible, !node->visible, node->visible));

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    //Transform properties tree
    // TODO: verificar por outros instantes na mesma posição antes de inserir
    if(ImGui::TreeNode("Transform Properties"))
    {
        if(ImGui::BeginTable("Transform Properties", 2,
        ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable ))
        {
            // Position
            
            ImGui::TableNextColumn();
            ImGui::Text("Position");
            ImGui::TableNextColumn();

            if(ImGui::BeginTable("PositionInput",2))
            {
                ImGui::TableSetupColumn("Input",ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Button",ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableNextColumn();

                float position[2] = {this->current_position.value()[0], this->current_position.value()[1]};
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat2("##PropertyPosition", position);
                this->current_position.value() = {position[0], position[1]};

                if (ImGui::IsItemDeactivatedAfterEdit())
                {   
                    history->push_action(std::make_unique<NodeMemberEdit<glm::vec2>>(node, node->position, node->position, this->current_position.value()));
                    node->position = this->current_position.value();
                }
                else if (ImGui::IsItemActive() == false)
                {
                    this->current_position = node->position;
                }

                ImGui::TableNextColumn();
                if(ImGui::Button("O##Position"))
                {
                    auto instant = Vector2Instant{anim_data.get_time(),glm::vec2(position[0],position[1]),"Linear"};
                    
                    node->keyframe.insert_instant<Track::POSITION>(instant);
                    history->push_action(std::make_unique<NodeKeyframeInsert<Track::POSITION>>(node, instant));
                }

                ImGui::EndTable();
            }

            
            
            // Scale

            ImGui::TableNextColumn();
            ImGui::Text("Scale");
            ImGui::TableNextColumn();

            
            if(ImGui::BeginTable("ScaleInput",2))
            {          
                ImGui::TableSetupColumn("Input",ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Button",ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableNextColumn();

                float scale[2] = {this->current_scale.value()[0], this->current_scale.value()[1]};
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat2("##PropertyScale", scale);
                this->current_scale = {scale[0], scale[1]};

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    history->push_action(std::make_unique<NodeMemberEdit<glm::vec2>>(node, node->scale, node->scale, this->current_scale.value()));
                    node->scale = this->current_scale.value();
                }
                else if (ImGui::IsItemActive() == false)
                {
                    this->current_scale = node->scale;
                }
                
                ImGui::TableNextColumn();
                if(ImGui::Button("O##Scale"))
                {
                    auto instant = Vector2Instant{anim_data.get_time(),glm::vec2(scale[0],scale[1]),"Linear"};

                    node->keyframe.insert_instant<Track::SCALE>(instant);
                    history->push_action(std::make_unique<NodeKeyframeInsert<Track::SCALE>>(node, instant));
                }

                ImGui::EndTable();
            }




            // Rotation Degrees
            
            ImGui::TableNextColumn();
            ImGui::Text("Rotation Degrees");
            ImGui::TableNextColumn();

            if(ImGui::BeginTable("RotationDegreesInput",2))
            {          
                ImGui::TableSetupColumn("Input",ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Button",ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableNextColumn();

                float rotation_deg = this->current_rotation_degrees.value();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat("##PropertyRotationDegrees", &rotation_deg,0.05, -rot_deg, +rot_deg);
                this->current_rotation_degrees = rotation_deg;

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    history->push_action(std::make_unique<NodeMemberEdit<double>>(node, node->rotation, node->rotation, glm::radians(this->current_rotation_degrees.value())));
                    node->rotation = glm::radians(this->current_rotation_degrees.value());
                }
                else if (ImGui::IsItemActive() == false)
                {
                    this->current_rotation_degrees = glm::degrees(node->rotation);
                }

                ImGui::TableNextColumn();
                if(ImGui::Button("O##RotationDegrees"))
                {
                    auto instant = DoubleInstant{anim_data.get_time(), node->rotation, "Linear"};

                    node->keyframe.insert_instant<Track::ROTATION>(instant);
                    history->push_action(std::make_unique<NodeKeyframeInsert<Track::ROTATION>>(node, instant));
                }

                ImGui::EndTable();

            }



            
            // Rotation

            ImGui::TableNextColumn();
            ImGui::Text("Rotation");
            ImGui::TableNextColumn();

            if(ImGui::BeginTable("RotationInput",2))
            {          
                ImGui::TableSetupColumn("Input",ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Button",ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableNextColumn();

                float rotation = this->current_rotation.value();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat("##PropertyRotation",&rotation,0.01,-rot_rad,rot_rad);
                this->current_rotation = rotation;

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    history->push_action(std::make_unique<NodeMemberEdit<double>>(node, node->rotation, node->rotation, this->current_rotation.value()));
                    node->rotation = this->current_rotation.value();
                }
                else if (ImGui::IsItemActive() == false)
                {
                    this->current_rotation = node->rotation;
                }



                ImGui::TableNextColumn();
                if(ImGui::Button("O##Rotation"))
                {
                    auto instant = DoubleInstant{anim_data.get_time(), node->rotation, "Linear"};

                    node->keyframe.insert_instant<Track::ROTATION>(instant);
                    history->push_action(std::make_unique<NodeKeyframeInsert<Track::ROTATION>>(node, instant));
                }
                ImGui::EndTable();
            }




            //  Rotation Pivot
            
            ImGui::TableNextColumn();
            ImGui::Text("Rotation Pivot");
            ImGui::TableNextColumn();

            if(ImGui::BeginTable("RotationPivotInput",2))
            {          
                ImGui::TableSetupColumn("Input",ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Button",ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableNextColumn();

                float pivot[2] = {this->current_rotation_pivot.value()[0], this->current_rotation_pivot.value()[1]};
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat2("##PropertyRotationPivot", pivot);
                this->current_rotation_pivot.value() = {pivot[0], pivot[1]};

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    history->push_action(std::make_unique<NodeMemberEdit<glm::vec2>>(node, node->rotation_pivot, node->rotation_pivot, this->current_rotation_pivot.value()));
                    node->rotation_pivot = this->current_rotation_pivot.value();
                }
                else if (ImGui::IsItemActive() == false)
                {
                    this->current_rotation_pivot = node->rotation_pivot;
                }

                ImGui::TableNextColumn();
                if(ImGui::Button("O##RotationPivot"))
                {
                    auto instant = Vector2Instant{anim_data.get_time(),glm::vec2(pivot[0],pivot[1]),"Linear"};

                    node->keyframe.insert_instant<Track::PIVOT>(instant);
                    history->push_action(std::make_unique<NodeKeyframeInsert<Track::PIVOT>>(node, instant));
                }
                ImGui::EndTable();
            }

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }

    //Visual properties tree
    if(ImGui::TreeNode("Visual Properties"))
    {
        if(ImGui::BeginTable("Visual Properties", 2,
        ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersOuterH))
        {
            ImGui::TableNextColumn();
            ImGui::Text("Layer");
            ImGui::TableNextColumn();

            int layer = this->current_layer.value();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputInt("##PropertyLayer", &layer);
            this->current_layer = layer;

            if (ImGui::IsItemDeactivatedAfterEdit())
            {
                history->push_action(std::make_unique<NodeMemberEdit<size_t>>(node, node->layer, node->layer, this->current_layer.value()));
                node->layer = this->current_layer.value();
            }
            else if (ImGui::IsItemActive() == false)
            {
                this->current_layer = node->layer;
            }

            ImGui::EndTable();
        }

        ImGui::TreePop();
    }
    
    ImGui::EndChild();
}

void PropertyEditor::reset_status()
{
    this->current_rotation_pivot = std::nullopt;
    this->current_rotation = std::nullopt;
    this->current_rotation_degrees = std::nullopt;
    this->current_scale = std::nullopt;
    this->current_position = std::nullopt;
    this->current_layer = std::nullopt;
}

void PropertyEditor::init_status(Node& node)
{
    this->current_rotation_pivot = node.rotation_pivot;
    this->current_rotation = node.rotation;
    this->current_rotation_degrees = glm::degrees(node.rotation);
    this->current_scale = node.scale;
    this->current_position = node.position;
    this->current_layer = node.layer;
}
