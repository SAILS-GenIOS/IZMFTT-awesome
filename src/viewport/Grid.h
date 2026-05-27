#pragma once

#include "gl_common.h"

#include <glm/glm.hpp>

namespace materializr {

/// Renders an infinite XZ ground grid using a full-screen quad shader technique.
class Grid {
public:
    Grid();
    ~Grid();

    /// Initialize the grid (compile shaders, create VAO).
    /// Call once after OpenGL context is ready.
    bool initialize();

    /// Render the grid.
    void render(const glm::mat4& view, const glm::mat4& projection,
                float nearPlane, float farPlane);

private:
    bool compileShader(unsigned int& shader, unsigned int type, const char* source);
    bool linkProgram(unsigned int vertShader, unsigned int fragShader);

    unsigned int m_shaderProgram = 0;
    unsigned int m_vao = 0; // Dummy VAO for the full-screen quad trick

    // Uniform locations
    int m_locViewProjection = -1;
    int m_locInvViewProjection = -1;
    int m_locNear = -1;
    int m_locFar = -1;
};

} // namespace materializr
