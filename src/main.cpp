#include "App.hpp"
#include "utilities.hpp"
#include <iostream>


class MyApp : public App{

public:
    MyApp() = default;
    ~MyApp() = default;

    virtual void startUp() final{

    }
    virtual void update() final{


        {
            ImGui::Begin("Properties");                          // Create a window called "Hello, world!" and append into it.

            float fps = ImGui::GetIO().Framerate;

            ImGui::Text("FPS: %f",fps);               // Display some text (you can use a format strings too)
            
            if(ImGui::IsKeyPressed(ImGuiKey_Enter)){
                
                std::cout << "Enter pressed" << std::endl;

            }

            ImGui::End();
        }


        {
            ImGui::Begin("Image");
            // Create a OpenGL texture identifier
            Image image;
            image.width = ImGui::GetContentRegionAvail().x;
            image.height = ImGui::GetContentRegionAvail().y;
            image.data = new unsigned char[image.width * image.height * 4];

            // Fill image with random pixels

            for (int i = 0; i < image.width * image.height * 4; i += 4)
            {
                image.data[i] = rand() % 255; // R
                image.data[i + 1] = rand() % 255; // G
                image.data[i + 2] = rand() % 255; // B
                image.data[i + 3] = 255; // A
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
    
    }
    virtual void render() final{

    }

private:
    bool show_demo_window = true;
    bool show_another_window = false;
    Image image;

public:


};

int main(int, char**){

    MyApp app;
    app.run();

    return 0;

}