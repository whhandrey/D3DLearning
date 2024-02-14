#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include <string>

struct WindowSize {
    int m_width;
    int m_height;
};

class Window {
public:
    Window(const WindowSize& size, const std::string& name);
    ~Window();

public:
    bool Initialize();
    void Run();

    bool IsVisible() const;
    WindowSize GetSize() const;

    HWND GetNativeHandle() const;

private:
    GLFWwindow* m_window;
    const WindowSize m_size;
    const std::string m_name;
};
