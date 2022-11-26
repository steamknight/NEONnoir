#define SIMPLE_IMAGE_IMPL
#include "simple_image.h"

#include "types.h"
#include "glfw_utils.h"

namespace NEONnoir
{
    GLtexture load_texture(MPG::simple_image const& image)
    {
        auto texture = GLtexture{};
        texture.width = image.width;
        texture.height = image.height;

        // Depalettize the image if needed
        auto depalettized = MPG::simple_image{ image };
        if (depalettized.bit_depth < 24)
        {
            depalettized = MPG::depalettize_image(depalettized);
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