#include "serialization.hpp"
#include "config.hpp"
#include "node_tree.hpp"
#include <chrono>

void serialize_project(const std::filesystem::path& path)
{
    //Update the current root node, and consequently all the node tree
    std::ofstream file{path};
    if (file.bad() || !file.is_open())
        panic(fmt::format("could not open file '{}'", path.string()));

    boost::archive::text_oarchive archive(file);
    
	archive << config.current_project;
    archive << node_tree;
    archive << anim_data;
}

void unload_project()
{
	delete node_tree;
	node_tree = new NodeTree{};
    config.current_project = ApplicationConfig::Project{};
    anim_data = AnimationData{};
}

void load_project(const std::filesystem::path& path)
{
    //Delete the current node tree and load the given one

    if (std::filesystem::exists(path) && !std::filesystem::is_regular_file(path))
        panic(fmt::format("file '{}' does not exist or is not a file", path.string()));
    std::ifstream file{path};

    if (file.bad() || !file.is_open())
        panic(fmt::format("could not open file '{}'", path.string()));
    boost::archive::text_iarchive archive(file);

	delete node_tree;
	archive >> config.current_project;
    archive >> node_tree;
    archive >> anim_data;
    config.current_project.header.last_access = boost::posix_time::second_clock::local_time();

}

template<class Archive>
void boost::serialization::serialize(Archive& archive,NodeTree& node_tree ,const unsigned int)
{
	archive & node_tree.root_node;
}

template<class Archive>
void boost::serialization::serialize(Archive& archive,Node& node ,const unsigned int)
{
    archive & node.name;
    archive & node.visible;

    archive & node.position;
    archive & node.scale;
    archive & node.rotation;
    archive & node.rotation_pivot;

    archive & node.layer;
    archive & node.texture_path;

    archive & node.parent;
    archive & node.children;
    archive & node.index;

    archive & node.keyframe;
}

template<class Archive>
void boost::serialization::serialize(Archive& archive, KeyFrame& keyframe, const unsigned int)
{
    archive & keyframe.position;
    archive & keyframe.rot_pivot;
    archive & keyframe.scale;

    archive & keyframe.rotation;
}

template<class Archive>
void boost::serialization::save(Archive& archive, const Vector2Instant& instant, const unsigned int)
{
    archive << instant.time;
    archive << instant.vector;
    archive << easings_ref.at(instant.easing);
}

template<class Archive>
void boost::serialization::load(Archive& archive, Vector2Instant& instant, const unsigned int)
{
    archive >> instant.time;
    archive >> instant.vector;

    std::string easing_name;
    archive >> easing_name;
    instant.easing = easings.at(easing_name);
}

template<class Archive>
void boost::serialization::save(Archive& archive, const DoubleInstant& instant, const unsigned int)
{
    archive << instant.time;
    archive << instant.value;
    archive << easings_ref.at(instant.easing);
}

template<class Archive>
void boost::serialization::load(Archive& archive, DoubleInstant& instant, const unsigned int)
{
    archive >> instant.time;
    archive >> instant.value;

    std::string easing_name;
    archive >> easing_name;
    instant.easing = easings.at(easing_name);
}


template<class Archive>
void boost::serialization::serialize(Archive& archive,ApplicationConfig::Project& project, const unsigned int)
{
    archive & project.header;
    archive & project.preferences;

}

BOOST_SERIALIZATION_SPLIT_FREE(ApplicationConfig::Project::Header);
template<class Archive>
void boost::serialization::save(Archive& archive,const ApplicationConfig::Project::Header& header, const unsigned int)
{
    archive << header.name;
    archive << header.path.string();
    archive << boost::posix_time::to_simple_string(header.last_access);
    archive << header.window_size.x;
    archive << header.window_size.y;
}

template<class Archive>
void boost::serialization::load(Archive& archive, ApplicationConfig::Project::Header& header, const unsigned int)
{
    archive >> header.name;
    
    std::string path;
    archive >> path;
    header.path = path;

    std::string time;
    archive >> time;
    header.last_access = boost::posix_time::time_from_string(time);

    archive >> header.window_size.x;
    archive >> header.window_size.y;
}

template<class Archive>
void boost::serialization::serialize(Archive& archive, ApplicationConfig::Project::Preferences& preferences, const unsigned int)
{
    archive & preferences.video_resolution;
}


//Third party
template<class Archive>
void boost::serialization::serialize(Archive& archive, glm::vec2& vec2, const unsigned int)
{
    archive & vec2.x;
    archive & vec2.y;
}

template<class Archive>
void boost::serialization::serialize(Archive& archive, ImVec4& vec4, const unsigned int)
{
    archive & vec4.w;
    archive & vec4.x;
    archive & vec4.y;
    archive & vec4.z;
}