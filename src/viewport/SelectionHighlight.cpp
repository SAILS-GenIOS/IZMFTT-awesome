#include "gl_common.h"
#include "SelectionHighlight.h"
#include "core/SelectionManager.h"
#include "core/Document.h"

#include <glm/gtc/type_ptr.hpp>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>

#include <vector>
#include <cstdio>

namespace materializr {

static const char* s_vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 a_position;
uniform mat4 u_mvp;
void main() {
    gl_Position = u_mvp * vec4(a_position, 1.0);
}
)";

static const char* s_fragSrc = R"(
#version 330 core
uniform vec4 u_color;
out vec4 fragColor;
void main() {
    fragColor = u_color;
}
)";

SelectionHighlight::SelectionHighlight() {}

SelectionHighlight::~SelectionHighlight() {
    if (m_program) glDeleteProgram(m_program);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}

bool SelectionHighlight::initialize() {
    unsigned int vert = 0, frag = 0;
    if (!compileShader(vert, GL_VERTEX_SHADER, s_vertSrc)) return false;
    if (!compileShader(frag, GL_FRAGMENT_SHADER, s_fragSrc)) {
        glDeleteShader(vert);
        return false;
    }
    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);
    glDeleteShader(vert);
    glDeleteShader(frag);

    int success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) return false;

    m_locMVP = glGetUniformLocation(m_program, "u_mvp");
    m_locColor = glGetUniformLocation(m_program, "u_color");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

void SelectionHighlight::render(const SelectionManager& sel, const Document& doc,
                                 const glm::mat4& view, const glm::mat4& projection) {
    if (!sel.hasSelection() || !m_program) return;

    glm::mat4 vp = projection * view;
    glm::vec3 faceColor(0.25f, 0.55f, 1.0f);
    glm::vec3 edgeColor(0.2f, 1.0f, 0.4f);
    glm::vec3 bodyOutlineColor(0.3f, 0.6f, 1.0f);

    for (const auto& entry : sel.getSelection()) {
        if (entry.shape.IsNull()) continue;

        switch (entry.type) {
            case SelectionType::Face:
                renderFace(entry.shape, vp, faceColor);
                break;
            case SelectionType::Edge:
                renderEdge(entry.shape, vp, edgeColor);
                break;
            case SelectionType::Body:
                renderBody(entry.shape, vp, bodyOutlineColor);
                break;
            default:
                break;
        }
    }
}

void SelectionHighlight::renderFace(const TopoDS_Shape& faceShape, const glm::mat4& vp,
                                     const glm::vec3& color) {
    // Just a blue tint over the face — no outline, no solid
    TopoDS_Face face = TopoDS::Face(faceShape);
    TopLoc_Location location;
    Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, location);
    if (tri.IsNull()) {
        BRepMesh_IncrementalMesh mesh(faceShape, 0.1);
        mesh.Perform();
        tri = BRep_Tool::Triangulation(face, location);
        if (tri.IsNull()) return;
    }

    const gp_Trsf& trsf = location.Transformation();
    bool hasXform = !location.IsIdentity();

    std::vector<float> verts;
    for (int i = 1; i <= tri->NbTriangles(); i++) {
        int n1, n2, n3;
        tri->Triangle(i).Get(n1, n2, n3);
        gp_Pnt p1 = tri->Node(n1), p2 = tri->Node(n2), p3 = tri->Node(n3);
        if (hasXform) { p1.Transform(trsf); p2.Transform(trsf); p3.Transform(trsf); }
        verts.insert(verts.end(), {(float)p1.X(),(float)p1.Y(),(float)p1.Z()});
        verts.insert(verts.end(), {(float)p2.X(),(float)p2.Y(),(float)p2.Z()});
        verts.insert(verts.end(), {(float)p3.X(),(float)p3.Y(),(float)p3.Z()});
    }
    if (verts.empty()) return;

    glUseProgram(m_program);
    glUniformMatrix4fv(m_locMVP, 1, GL_FALSE, glm::value_ptr(vp));
    glUniform4f(m_locColor, color.r, color.g, color.b, 0.35f);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(verts.size() / 3));

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
    glUseProgram(0);
}

void SelectionHighlight::renderEdge(const TopoDS_Shape& edgeShape, const glm::mat4& vp,
                                     const glm::vec3& color) {
    try {
        TopoDS_Edge edge = TopoDS::Edge(edgeShape);
        BRepAdaptor_Curve curve(edge);
        GCPnts_TangentialDeflection discretizer(curve, 0.05, 0.05);
        int nPts = discretizer.NbPoints();
        if (nPts < 2) return;

        std::vector<float> verts;
        for (int i = 1; i < nPts; i++) {
            gp_Pnt p1 = discretizer.Value(i);
            gp_Pnt p2 = discretizer.Value(i + 1);
            verts.insert(verts.end(), {(float)p1.X(),(float)p1.Y(),(float)p1.Z()});
            verts.insert(verts.end(), {(float)p2.X(),(float)p2.Y(),(float)p2.Z()});
        }

        glUseProgram(m_program);
        glUniformMatrix4fv(m_locMVP, 1, GL_FALSE, glm::value_ptr(vp));
        glUniform4f(m_locColor, color.r, color.g, color.b, 1.0f);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

        glDisable(GL_DEPTH_TEST);
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, static_cast<int>(verts.size() / 3));
        glLineWidth(1.0f);
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(0);
        glUseProgram(0);
    } catch (...) {}
}

void SelectionHighlight::renderBody(const TopoDS_Shape& bodyShape, const glm::mat4& vp,
                                     const glm::vec3& color) {
    // Render all edges of the body in highlight color
    std::vector<float> verts;
    for (TopExp_Explorer exp(bodyShape, TopAbs_EDGE); exp.More(); exp.Next()) {
        try {
            TopoDS_Edge edge = TopoDS::Edge(exp.Current());
            BRepAdaptor_Curve curve(edge);
            GCPnts_TangentialDeflection discretizer(curve, 0.1, 0.1);
            int nPts = discretizer.NbPoints();
            for (int i = 1; i < nPts; i++) {
                gp_Pnt p1 = discretizer.Value(i);
                gp_Pnt p2 = discretizer.Value(i + 1);
                verts.insert(verts.end(), {(float)p1.X(),(float)p1.Y(),(float)p1.Z()});
                verts.insert(verts.end(), {(float)p2.X(),(float)p2.Y(),(float)p2.Z()});
            }
        } catch (...) { continue; }
    }
    if (verts.empty()) return;

    glUseProgram(m_program);
    glUniformMatrix4fv(m_locMVP, 1, GL_FALSE, glm::value_ptr(vp));
    glUniform4f(m_locColor, color.r, color.g, color.b, 1.0f);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_DYNAMIC_DRAW);

    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.5f);
    glDrawArrays(GL_LINES, 0, static_cast<int>(verts.size() / 3));
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
    glUseProgram(0);
}

bool SelectionHighlight::compileShader(unsigned int& shader, unsigned int type, const char* source) {
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(shader);
        shader = 0;
        return false;
    }
    return true;
}

} // namespace materializr
