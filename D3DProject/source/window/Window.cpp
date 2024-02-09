#include "Window.h"
#include <iostream>

Window::Window(const WindowSize& size, const std::string& name)
    : m_size{ size }
    , m_name{ name }
{
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
}

bool Window::Initialize()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_size.m_width, m_size.m_height, m_name.c_str(), nullptr, nullptr);

    return m_window != nullptr;
}

void Window::Run()
{
    glfwPollEvents();
}

bool Window::IsVisible() const
{
    return !glfwWindowShouldClose(m_window);
}

WindowSize Window::GetSize() const
{
    return m_size;
}
