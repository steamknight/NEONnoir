#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <string_view>
#include <exception>
#include <memory>

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

    struct GLtexture
    {
        GLuint texture_id{};
        int32_t width{};
        int32_t height{};
        operator void* () { return (void*)(intptr_t)texture_id; }
    };

    GLtexture load_texture(std::string_view const& image_path);
}