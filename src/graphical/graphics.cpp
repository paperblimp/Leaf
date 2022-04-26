// header
#include "graphics.hpp"

// local
#include "config.hpp"
#include "utils/asserts.hpp"



void GraphicContext::init()
{      
    leaf_assert(this->initialized == false);

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        panic("glfw launch error");



    // GL 3.0 + GLSL 130
    this->glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

    // window and context creation
    this->window = glfwCreateWindow(1280, 720, "Leaf", NULL, NULL);

    if (window == nullptr)
        panic("error while creating window");


    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    this->export_window = glfwCreateWindow(640, 480, "", NULL, NULL);

    if (this->export_window == nullptr)
        panic("error while creating export window");


    glfwMakeContextCurrent(this->window);



    auto version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
        panic("failed  to initialize opengl context");
    else
        notice(fmt::format("opengl version: {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version)));

    // enable vsync
    glfwSwapInterval(config.graphic_config.vsync);
            

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    this->imgui_io = &ImGui::GetIO();

    // imgui attributes
    this->imgui_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    this->imgui_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // init imgui backends
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.value().data());


    leaf_assert(this->glsl_version.has_value());
    leaf_assert(this->window != nullptr);
    leaf_assert(this->imgui_io != nullptr);
    
    this->initialized = true;
}


void GraphicContext::destroy()
{
    leaf_assert(this->initialized == true);

    // destroy imgui context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // destroy opengl/SDL context
    glfwDestroyWindow(this->window);
    glfwDestroyWindow(this->export_window);
    glfwTerminate();

    this->initialized = false;
}


void GraphicContext::start_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GraphicContext::end_frame(ImVec4 clear_color)
{
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(graphic_context.window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GraphicContext::display_frame()
{
    glfwSwapBuffers(this->window);
}


void GraphicContext::make_current_main_context()
{
    glfwMakeContextCurrent(this->window);
}

void GraphicContext::make_current_export_context()
{
    glfwMakeContextCurrent(this->export_window);
}



GraphicContext::~GraphicContext()
{
    leaf_assert(this->initialized == false);
}