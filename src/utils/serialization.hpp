#pragma once

// builtin
#include <string.h>
#include <optional>
#include <fstream>
#include <chrono>
#include <filesystem>

// extern
#include <glm/ext/vector_float2.hpp>
#include <imgui.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>

// local
#include "node_tree.hpp"
#include "animation/easings.hpp"
#include "animation/keyframe.hpp"
#include "config.hpp"
#include "animation/animation.hpp"



void serialize_project(const std::filesystem::path& path);
void unload_project();
void load_project(const std::filesystem::path& path);


namespace boost::serialization {

    template<class Archive>
    void serialize(Archive& archive,NodeTree& node_tree ,const unsigned int _version);

    template<class Archive>
    void serialize(Archive& archive,Node& node ,const unsigned int _version);

    template<class Archive>
    void serialize(Archive& archive, KeyFrame& keyframe, const unsigned int _version);

    
    template<class Archive>
    void save(Archive& archive, const Vector2Instant& instant, const unsigned int _version);

    template<class Archive>
    void load(Archive& archive, Vector2Instant& instant, const unsigned int _version);
    

    
    template<class Archive>
    void save(Archive& archive, const DoubleInstant& instant, const unsigned int _version);

    template<class Archive>
    void load(Archive& archive, DoubleInstant& instant, const unsigned int _version);

    
    
    template<class Archive>
    void serialize(Archive& archive, ApplicationConfig::Project& project, const unsigned int _version);

    template<class Archive>
    void save(Archive& archive,const ApplicationConfig::Project::Header& header, const unsigned int _version);
    template<class Archive>
    void load(Archive& archive, ApplicationConfig::Project::Header& header, const unsigned int _version);

    template<class Archive>
    void serialize(Archive& archive, ApplicationConfig::Project::Preferences& preferences, const unsigned int _version);

    template<class Archive>
    void serialize(Archive& archive, glm::vec2& vec2, const unsigned int _version);

    template<class Archive>
    void serialize(Archive& archive, ImVec4& vec4, const unsigned int _version);

}

BOOST_SERIALIZATION_SPLIT_FREE(DoubleInstant);
BOOST_SERIALIZATION_SPLIT_FREE(Vector2Instant);
