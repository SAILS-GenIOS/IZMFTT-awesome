#pragma once
#include "gl_common.h"
#include <glm/glm.hpp>
#include <TopoDS_Shape.hxx>

class SelectionManager;
class Document;

namespace materializr {

class SelectionHighlight {
public:
    SelectionHighlight();
    ~SelectionHighlight();

    bool initialize();

    void render(const SelectionManager& sel, const Document& doc,
                const glm::mat4& view, const glm::mat4& projection);

private:
    void renderFace(const TopoDS_Shape& face, const glm::mat4& vp, const glm::vec3& color);
    void renderEdge(const TopoDS_Shape& edge, const glm::mat4& vp, const glm::vec3& color);
    void renderBody(const TopoDS_Shape& body, const glm::mat4& vp, const glm::vec3& color);

    bool compileShader(unsigned int& shader, unsigned int type, const char* source);

    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_locMVP = -1;
    int m_locColor = -1;
};

} // namespace materializr
