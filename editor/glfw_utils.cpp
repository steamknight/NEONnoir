#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SIMPLE_IMAGE_IMPL
#include "simple_image.h"

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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        // Upload pixels into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        stbi_image_free(image_data);

        return texture;
    }

    GLtexture load_texture(MPG::simple_image const& image)
    {
        auto texture = GLtexture{};
        texture.width = image.width;
        texture.height = image.height;

        // Depalettize the image if needed
        auto depalettized = MPG::simple_image{ image };
        if (depalettized.bit_depth < 24)
        {
            depalettized = MPG::depalettize_8bit_image(depalettized);
        }

        // Create a OpenGL texture identifier
        glGenTextures(1, &texture.texture_id);
        glBindTexture(GL_TEXTURE_2D, texture.texture_id);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // Upload pixels into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, depalettized.pixel_data.data());

        return texture;
    }

    void free_texture(GLtexture& texture)
    {
        glDeleteTextures(1, &texture.texture_id);
    }
}