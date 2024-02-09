#pragma once

class glfwAutoInit {
public:
    glfwAutoInit();
    ~glfwAutoInit();

public:
    bool Initialized() const;

private:
    bool m_initialized = false;
};
