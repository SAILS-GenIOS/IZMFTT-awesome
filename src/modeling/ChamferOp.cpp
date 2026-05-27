#include "ChamferOp.h"
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopoDS.hxx>
#include <imgui.h>

ChamferOp::ChamferOp() = default;

void ChamferOp::setBody(int bodyId) {
    m_bodyId = bodyId;
}

void ChamferOp::setEdges(const std::vector<TopoDS_Edge>& edges) {
    m_edges = edges;
}

void ChamferOp::setDistance(double distance) {
    m_distance = distance;
}

bool ChamferOp::execute(Document& doc) {
    if (m_bodyId < 0 || m_edges.empty() || m_distance <= 0.0) {
        return false;
    }

    try {
        // Store previous shape for undo
        m_previousShape = doc.getBody(m_bodyId);

        // Build an edge-face map so we can find a face adjacent to each edge
        TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
        TopExp::MapShapesAndAncestors(m_previousShape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);

        // Create chamfer on the body shape
        BRepFilletAPI_MakeChamfer chamfer(m_previousShape);

        for (const auto& edge : m_edges) {
            // Find a face adjacent to this edge
            if (edgeFaceMap.Contains(edge)) {
                const TopTools_ListOfShape& faces = edgeFaceMap.FindFromKey(edge);
                if (!faces.IsEmpty()) {
                    const TopoDS_Face& face = TopoDS::Face(faces.First());
                    chamfer.Add(m_distance, m_distance, edge, face);
                }
            }
        }

        chamfer.Build();
        if (!chamfer.IsDone()) {
            return false;
        }

        // Update the body with the chamfered shape
        doc.updateBody(m_bodyId, chamfer.Shape());
        return true;
    } catch (...) {
        return false;
    }
}

bool ChamferOp::undo(Document& doc) {
    if (m_bodyId < 0 || m_previousShape.IsNull()) {
        return false;
    }

    try {
        doc.updateBody(m_bodyId, m_previousShape);
        return true;
    } catch (...) {
        return false;
    }
}

std::string ChamferOp::description() const {
    return "Chamfer D" + std::to_string(m_distance) + " on " +
           std::to_string(m_edges.size()) + " edge(s)";
}

void ChamferOp::renderProperties() {
    ImGui::Text("Chamfer");
    ImGui::Separator();

    ImGui::InputDouble("Distance", &m_distance, 0.1, 1.0, "%.3f");

    ImGui::Text("Edges: %d selected", static_cast<int>(m_edges.size()));
    ImGui::Text("Body ID: %d", m_bodyId);
}
