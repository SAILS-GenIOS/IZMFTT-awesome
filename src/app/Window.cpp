#include "app/Window.h"

#include "gl_common.h"   // GLEW (Windows) must be included before GLFW
#include <GLFW/glfw3.h>
#include "app/IconData.h"
#include <stdexcept>
#include <iostream>

namespace materializr {

void Window::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height) {

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Tell the window manager who we are. Without these, GLFW reports
    // WM_CLASS = "glfw" on X11 and app-id = "glfw" on Wayland, so taskbar
    // extensions (Dash-to-Panel, the GNOME shell, etc.) can't match the
    // running window to materializr.desktop and fall back to a generic icon.
    // StartupWMClass=Materializr in the desktop file pairs with this.
    glfwWindowHintString(GLFW_X11_CLASS_NAME,    "Materializr");
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "Materializr");
    glfwWindowHintString(GLFW_WAYLAND_APP_ID,    "Materializr");

    m_window = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Window / taskbar icon from embedded pixels — this is what makes the
    // Windows taskbar (and X11 docks) show the logo instead of the generic
    // exe icon. Wayland ignores runtime icons (it uses the .desktop entry,
    // which the AppImage already provides). The .rc-embedded .ico covers
    // Explorer on Windows.
    {
        GLFWimage icons[2];
        icons[0].width  = materializr_icon::kSize48;
        icons[0].height = materializr_icon::kSize48;
        icons[0].pixels = const_cast<unsigned char*>(materializr_icon::kPixels48);
        icons[1].width  = materializr_icon::kSize32;
        icons[1].height = materializr_icon::kSize32;
        icons[1].pixels = const_cast<unsigned char*>(materializr_icon::kPixels32);
        glfwSetWindowIcon(m_window, 2, icons);
    }

    glfwMakeContextCurrent(m_window);

#ifdef _WIN32
    // Load GL 3.3 core entry points (no-op on Linux, which uses Mesa prototypes).
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLEW (OpenGL loader)");
    }
#endif

    glfwSwapInterval(1); // Enable vsync
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

} // namespace materializr
