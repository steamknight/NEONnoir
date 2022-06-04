#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glfw_utils.h"

namespace NEONnoir
{
    GLtexture load_texture(std::string_view const& image_path)
    {
        auto texture = GLtexture{};

        // Load from file
        uint8_t* image_data = stbi_load(image_path.data(), &texture.width, &texture.height, nullptr, 4);
        //if (image_data == NULL)
        //    return nullptr;

        // Create a OpenGL texture identifier
        glGenTextures(1, &texture.texture_id);
        glBindTexture(GL_TEXTURE_2D, texture.texture_id);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        // Upload pixels into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);

        return texture;
    }
}