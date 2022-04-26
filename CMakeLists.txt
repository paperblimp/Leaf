cmake_minimum_required(VERSION 3.19)


cmake_policy(SET CMP0091 NEW)
project(Leaf LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(leaf 
    
    src/main.cpp
    src/node_tree.cpp
    src/history.cpp
    
    src/screens/main_screen.cpp
    src/screens/projects_screen.cpp
   
    src/utils/log.cpp
    src/utils/asserts.cpp
    src/utils/file_io.cpp
    src/utils/source_location.cpp
    src/utils/system.cpp
    
    src/dialogs/file_browser.cpp
    src/dialogs/text_input.cpp
    src/dialogs/choise.cpp
    src/dialogs/alert.cpp
    src/dialogs/preferences.cpp
    src/dialogs/export.cpp
    src/dialogs/frame_export.cpp

    src/sections/main_bar.cpp
    src/sections/section.cpp
    src/sections/property_editor.cpp
    src/sections/preview.cpp
    src/sections/filesystem.cpp
    src/sections/node_renderer.cpp
    src/sections/keyframe_widget.cpp
    src/sections/viewport.cpp
    src/sections/action_bar.cpp
    src/sections/properties_and_preview.hpp
    src/sections/depth_indicator.cpp

    src/graphical/opengl/render.cpp
    src/graphical/theme.cpp
    src/graphical/graphics.cpp
    src/graphical/sprite.cpp
    src/graphical/custom_widgets.cpp

    src/utils/asserts.cpp
    src/utils/conversions.hpp
    src/utils/file_io.cpp
    src/utils/source_location.cpp
    src/utils/system.cpp
    src/utils/serialization.cpp
    
    src/animation/animation.cpp
    src/animation/keyframe.cpp
    
    lib/glad/gl.c
)

set_property( TARGET leaf PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:DEBUG>:Debug>DLL")

# libraries

set_target_properties(leaf PROPERTIES
  LINK_SEARCH_START_STATIC ON
  LINK_SEARCH_END_STATIC ON
)

find_package(glfw3 CONFIG REQUIRED)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(leaf PRIVATE imgui::imgui)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(leaf PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(leaf PRIVATE fmt::fmt)

find_package(termcolor CONFIG REQUIRED)
target_link_libraries(leaf PRIVATE termcolor::termcolor)

find_package(glm CONFIG REQUIRED)
target_link_libraries(leaf PRIVATE glm::glm)

find_path(STB_INCLUDE_DIRS "stb.h")
target_include_directories(leaf PRIVATE ${STB_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS serialization filesystem)
target_link_libraries(leaf PRIVATE Boost::boost Boost::serialization Boost::filesystem)

find_package(FFMPEG REQUIRED)
target_include_directories(leaf PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(leaf PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(leaf PRIVATE ${FFMPEG_LIBRARIES})


if (UNIX)
    # target_link_libraries(leaf PRIVATE GL)
elseif(WIN32)    
    target_link_libraries(leaf PRIVATE opengl32 gdi32 imm32)
else()
    message(FATAL_ERROR "not implemented")
endif()

target_include_directories(leaf PRIVATE src lib)

message("${CMAKE_CXX_COMPILER_ID}")

# flags
   
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
    
    target_compile_options(leaf PRIVATE
        -std=c++17
        -pedantic
        -Wall
        -Wextra

        -fdiagnostics-color=always

        $<$<CONFIG:DEBUG>:-g>
        $<$<CONFIG:ASAN>:-g>
        $<$<CONFIG:TSAN>:-g>
        
        $<$<CONFIG:RELEASE>:-DNDEBUG>

        $<$<CONFIG:RELEASE>:-O3>
        $<$<CONFIG:RELEASE>:-flto>
        $<$<CONFIG:RELEASE>:-march=native>
    )
        
    target_link_libraries(leaf PRIVATE

        -static-libgcc
        -static-libstdc++
	    $<$<CONFIG:RELEASE>:-flto>

        $<$<CONFIG:ASAN>:-fsanitize=address>
        $<$<CONFIG:ASAN>:-fsanitize=undefined>
        $<$<CONFIG:TSAN>:-fsanitize=thread>
    )

else()
        
    target_compile_options(leaf PRIVATE
        /std:c++17
        /W2


        $<$<CONFIG:DEBUG>:/Z7>
        $<$<CONFIG:ASAN>:/Z7>
        $<$<CONFIG:TSAN>:/Z7>
        
        $<$<CONFIG:RELEASE>:/DNDEBUG>

        $<$<CONFIG:RELEASE>:/O2>
    )
        
    target_link_libraries(leaf PRIVATE

        $<$<CONFIG:ASAN>:/fsanitize=address>
        $<$<CONFIG:ASAN>:/fsanitize=undefined>
        $<$<CONFIG:TSAN>:/fsanitize=thread>
    )

endif()
