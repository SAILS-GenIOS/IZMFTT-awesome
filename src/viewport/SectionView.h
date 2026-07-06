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

    // Filled cross-section caps: without a cap a clipped solid looks hollow.
    // One entry per intersected body, carrying its material colour.
    struct CapMesh {
        std::vector<float> positions; // x,y,z per vertex, TRIANGLES
        glm::vec3 color;
    };
    std::vector<CapMesh> m_caps;
    glm::vec3 m_capNormal = glm::vec3(0.0f, 0.0f, 1.0f); // cut-plane normal (world)

    unsigned int m_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_locMVP = -1;
    int m_locColor = -1;

    // Cap fill program (flat-shaded so the cut reads as solid material).
    unsigned int m_capProgram = 0;
    unsigned int m_capVao = 0;
    unsigned int m_capVbo = 0;
    int m_capLocMVP = -1;
    int m_capLocColor = -1;
    int m_capLocNormal = -1;

    bool compileShader(unsigned int& shader, unsigned int type, const char* source);
};

} // namespace materializr
