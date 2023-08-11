

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <thread>
#include <vector>
#include <cmath>

// Define the number of threads to use
#define NUM_THREADS 4

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


// Image struct
struct Image
{
    unsigned char* data;
    int width;
    int height;
};

ImVec4 interpolateColors(const ImVec4& color1, const ImVec4& color2, double t) {
    ImVec4 result;
    result.w = static_cast<float>(1.0f);
    result.x = static_cast<float>(color1.x + t * (color2.x - color1.x));
    result.y = static_cast<float>(color1.y + t * (color2.y - color1.y));
    result.z = static_cast<float>(color1.z + t * (color2.z - color1.z));
    return result;
}

ImVec4 color1 = ImVec4(0.0f, 0.0f, 1.0f, 1.00f);
ImVec4 color2 = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);


inline void EnableDockSpace(bool *p_open);
inline void write_data(Image image, int r,int g,int b, int alpha, int i,int j);
void generate_colors(Image& image, int start_row, int end_row,int radius, ImVec4 circle_color);


// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Image Editor", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;        // Enable Docking
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);



    // Our state
    bool enable_dockspace = true;
    int radius = 10;
    std::thread threads[NUM_THREADS];

    // Circle Color
    ImVec4 circle_color = ImVec4(1.0f, 0.0f, 0.0f, 1.00f);

    // Background color
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        EnableDockSpace(&enable_dockspace);

        {
            ImGui::Begin("Properties");                          // Create a window called "Hello, world!" and append into it.

            float fps = ImGui::GetIO().Framerate;

            ImGui::Text("FPS: %f",fps);               // Display some text (you can use a format strings too)

            ImGui::SliderInt("Radius", &radius, 0, 500);            // Edit 1 float using a slider from 0.0f to 1.0f

            ImGui::ColorEdit3("Circle Color", (float*)&circle_color); // Edit 3 floats representing a color

            ImGui::ColorEdit3("Top Background Color", (float*)&color1); // Edit 3 floats representing a color")

            ImGui::ColorEdit3("Bottom Background Color", (float*)&color2); // Edit 3 floats representing a color")

            ImGui::End();
        }

        {
            ImGui::Begin("Image");
            // Create a OpenGL texture identifier
            Image image;
            image.width = ImGui::GetWindowWidth() - 20;
            image.height = ImGui::GetWindowHeight() - 40;
            image.data = new unsigned char[image.width * image.height * 4];

            // Start the color generation threads
            
            int rows_per_thread = image.height / NUM_THREADS;
            for (int i = 0; i < NUM_THREADS; ++i) {
                int start_row = i * rows_per_thread;
                int end_row = (i == NUM_THREADS - 1) ? image.height : (i + 1) * rows_per_thread;
                threads[i] = std::thread(generate_colors, std::ref(image), start_row, end_row, radius, circle_color);
            }

            // Wait for all threads to finish
            for (auto& thread : threads) {
                thread.join();
            }

            
            // Create a OpenGL texture identifier
            GLuint image_texture;
            glGenTextures(1, &image_texture);
            glBindTexture(GL_TEXTURE_2D, image_texture);

            // Setup filtering parameters for display
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same


            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data);

            ImGui::Image((void*)(intptr_t)image_texture, ImVec2(image.width, image.height));

            delete[] image.data;
            
            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


inline void EnableDockSpace(bool* p_open)
{
    
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if (!opt_padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", p_open, window_flags);
    if (!opt_padding)
        ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Options"))
        {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::Separator();

            if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
                *p_open = false;
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}


inline void write_data(Image image, int r,int g,int b, int alpha, int i,int j){


    image.data[(j * image.width + i) * 4 + 0] = r;
    image.data[(j * image.width + i) * 4 + 1] = g;
    image.data[(j * image.width + i) * 4 + 2] = b;
    image.data[(j * image.width + i) * 4 + 3] = alpha;
    
}



void generate_colors(Image& image, int start_row, int end_row,int radius, ImVec4 circle_color) {
    
    

    int r = static_cast<int>(circle_color.x * 255.0f);
    int g = static_cast<int>(circle_color.y * 255.0f);
    int b = static_cast<int>(circle_color.z * 255.0f);
    
    for (int j = start_row; j < end_row; ++j) {
        for (int i = 0; i < image.width; ++i) {

            if((i - image.width/2 )*(i - image.width/2) + (j - image.height/2)*(j - image.height/2) <= radius * radius)
                write_data(image, r, g, b, 255, i, j);

            else{
                double t = static_cast<double>(j) / (image.height - 1);  // Adjusted for 0-based indexing
                ImVec4 interpolatedColor = interpolateColors(color1, color2, t);

                write_data(image, static_cast<int>(interpolatedColor.x*255.0f), static_cast<int>(interpolatedColor.y*255.0f), static_cast<int>(interpolatedColor.z*255.0f), 255, i, j);
            }
        }
    }
}