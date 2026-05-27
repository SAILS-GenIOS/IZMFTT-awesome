#pragma once
#include "gl_common.h"
#include <glm/glm.hpp>
#include <TopoDS_Shape.hxx>
#include <vector>

namespace materializr {

class EdgeRenderer {
public:
    EdgeRenderer();
    ~EdgeRenderer();

    /// Initialize shaders. Call once after OpenGL context is ready.
    bool initialize();

    /// Extract and buffer edges from a shape.
    /// Returns the mesh index for later reference.
    int addShape(const TopoDS_Shape& shape, float deflection = 0.1f);

    /// Render all edge meshes as dark lines.
    void render(const glm::mat4& view, const glm::mat4& projection);

    /// Remove all edge meshes.
    void clear();

private:
    struct EdgeMesh {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        int vertexCount = 0;
    };

    bool compileShader(unsigned int& shader, unsigned int type, const char* source);

    std::vector<EdgeMesh> m_meshes;
    unsigned int m_program = 0;
    int m_locMVP = -1;
    int m_locColor = -1;
};

} // namespace materializr
