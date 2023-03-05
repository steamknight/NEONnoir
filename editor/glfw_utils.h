#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <string_view>
#include <exception>
#include <memory>
#include "simple_image.h"
#include "types.h"

namespace NEONnoir
{
    class glfw_error : public std::exception
    {
    public:
        glfw_error(std::string const& message)
        {
            char const* glfw_error = nullptr;
            glfwGetError(&glfw_error);

            _message = message + ": " + std::string{ glfw_error };
        }

        virtual const char* what() const noexcept
        {
            return _message.c_str();
        }

    private:
        std::string _message;
    };

    template<typename T>
    struct glfw_ptr_deleter
    {
        void operator()(T*) const noexcept {}
    };

    template<>
    inline void glfw_ptr_deleter<GLFWwindow>::operator()(GLFWwindow* pointer) const noexcept
    {
        if (!pointer) return;
        glfwDestroyWindow(pointer);
    }

    template<>
    inline void glfw_ptr_deleter<GLFWcursor>::operator()(GLFWcursor* pointer) const noexcept
    {
        if (!pointer) return;
        glfwDestroyCursor(pointer);
    }

    template<typename T>
    void ensure_valid(T* ptr, std::string const& error)
    {
        if (ptr == nullptr)
        {
            throw glfw_error(error);
        }
    }

    template<typename T>
    using glfw_ptr = std::unique_ptr<T, glfw_ptr_deleter<T>>;

    using GLFWwindow_ptr = glfw_ptr<GLFWwindow>;
    using GLFWcursor_ptr = glfw_ptr<GLFWcursor>;

    struct GLtexture
    {
        GLuint texture_id{};
        i32 width{};
        i32 height{};
        operator void* () { return (void*)(intptr_t)texture_id; }
    };

    GLtexture load_texture(MPG::simple_image const& image);
    void free_texture(GLtexture& texture);
}