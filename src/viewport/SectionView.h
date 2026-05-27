#pragma once
#include "gl_common.h"
#include <glm/glm.hpp>
#include <gp_Pln.hxx>
#include <TopoDS_Shape.hxx>
#include <vector>

class Document;

namespace materializr {

class SectionView {
public:
    SectionView();
    ~SectionView();

    bool initialize();

    void setDocument(const Document* doc);
    void setPlane(const gp_Pln& plane);
    void setOffset(float offset);
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // Compute section curves
    void update();

    // Render section lines in viewport
    void render(const glm::mat4& view, const glm::mat4& projection);

private:
    const Document* m_document = nullptr;
    gp_Pln m_plane;
    float m_offset = 0.0f;
    bool m_enabled = false;

    struct SectionLine {
        glm::vec3 start;
        glm::vec3 end;
    };
    std::vector<SectionLine> m_lines;

    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_locMVP = -1;
    int m_locColor = -1;

    bool compileShader(unsigned int& shader, unsigned int type, const char* source);
};

} // namespace materializr
