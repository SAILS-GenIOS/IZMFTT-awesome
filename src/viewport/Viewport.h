#pragma once

#include "gl_common.h"

#include "Camera.h"

#include <glm/glm.hpp>

namespace materializr {

/// Manages the 3D viewport FBO and input within an ImGui window.
class Viewport {
public:
    Viewport();
    ~Viewport();

    /// Recreate the framebuffer at a new size.
    void resize(int width, int height);

    /// Bind the viewport FBO for off-screen rendering.
    void bind();

    /// Unbind the viewport FBO (restore default framebuffer).
    void unbind();

    /// Get the color texture ID for ImGui::Image().
    unsigned int getTextureID() const { return m_colorTexture; }

    /// Get the current viewport dimensions.
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    /// Render the viewport as an ImGui window. Handles mouse input.
    void renderImGuiWindow();

    /// Access the camera.
    Camera& getCamera() { return m_camera; }
    const Camera& getCamera() const { return m_camera; }

private:
    void createFramebuffer();
    void destroyFramebuffer();
    void handleInput();

    Camera m_camera;

    unsigned int m_fbo = 0;
    unsigned int m_colorTexture = 0;
    unsigned int m_depthRenderbuffer = 0;

    int m_width = 1280;
    int m_height = 720;

    // Input state
    bool m_isHovered = false;
    glm::vec2 m_lastMousePos = glm::vec2(0.0f);
    bool m_isDragging = false;
};

} // namespace materializr
