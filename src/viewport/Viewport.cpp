#include "gl_common.h"

#include "Viewport.h"

#include <imgui.h>

namespace materializr {

Viewport::Viewport()
{
    createFramebuffer();
}

Viewport::~Viewport()
{
    destroyFramebuffer();
}

void Viewport::resize(int width, int height)
{
    if (width == m_width && height == m_height) return;
    if (width <= 0 || height <= 0) return;

    m_width = width;
    m_height = height;
    m_camera.setAspect(static_cast<float>(width) / static_cast<float>(height));

    destroyFramebuffer();
    createFramebuffer();
}

void Viewport::bind()
{
    // NOTE: glBindFramebuffer requires glad or GL 3.0+ function pointer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Viewport::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::renderImGuiWindow()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    // Check if the viewport content area has changed size
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    int newWidth = static_cast<int>(contentSize.x);
    int newHeight = static_cast<int>(contentSize.y);

    if (newWidth > 0 && newHeight > 0) {
        resize(newWidth, newHeight);
    }

    // Display the FBO color texture
    ImGui::Image(
        static_cast<ImTextureID>(m_colorTexture),
        contentSize,
        ImVec2(0, 1), // UV0: flip Y for OpenGL
        ImVec2(1, 0)  // UV1
    );

    // Handle mouse input when hovered
    m_isHovered = ImGui::IsItemHovered();
    handleInput();

    ImGui::End();
    ImGui::PopStyleVar();
}

void Viewport::handleInput()
{
    if (!m_isHovered) {
        m_isDragging = false;
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    glm::vec2 mousePos(io.MousePos.x, io.MousePos.y);
    glm::vec2 mouseDelta = mousePos - m_lastMousePos;

    // Scroll wheel -> zoom
    if (io.MouseWheel != 0.0f) {
        m_camera.zoom(io.MouseWheel);
    }

    // Middle mouse button interactions
    bool middleDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);

    if (middleDown) {
        if (io.KeyShift) {
            // Shift + middle mouse drag -> pan
            m_camera.pan(mouseDelta.x, mouseDelta.y);
        } else {
            // Middle mouse drag -> orbit
            m_camera.orbit(mouseDelta.x, mouseDelta.y);
        }
        m_isDragging = true;
    } else {
        m_isDragging = false;
    }

    m_lastMousePos = mousePos;
}

void Viewport::createFramebuffer()
{
    // NOTE: These calls require glad or GL 3.0+ extension loading.

    // Create framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Create color attachment texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, m_colorTexture, 0);

    // Create depth+stencil renderbuffer
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_depthRenderbuffer);

    // Check completeness
    // GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    // assert(status == GL_FRAMEBUFFER_COMPLETE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::destroyFramebuffer()
{
    if (m_colorTexture) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthRenderbuffer) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

} // namespace materializr
