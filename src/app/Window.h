#pragma once

#include <string>

struct GLFWwindow;

namespace materializr {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Non-copyable, non-movable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();

    GLFWwindow* handle() const { return m_window; }
    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    GLFWwindow* m_window = nullptr;
    int m_width;
    int m_height;

    static void errorCallback(int error, const char* description);
};

} // namespace materializr
