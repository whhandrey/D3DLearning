#include "glfwAutoInit.h"

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

glfwAutoInit::glfwAutoInit()
{
    if (!glfwInit())
        return;

    m_initialized = true;
}

glfwAutoInit::~glfwAutoInit()
{
    glfwTerminate();
}

bool glfwAutoInit::Initialized() const
{
    return m_initialized;
}
