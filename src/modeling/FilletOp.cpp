#include "FilletOp.h"
#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopoDS.hxx>
#include <imgui.h>

FilletOp::FilletOp() = default;

void FilletOp::setBody(int bodyId) {
    m_bodyId = bodyId;
}

void FilletOp::setEdges(const std::vector<TopoDS_Edge>& edges) {
    m_edges = edges;
}

void FilletOp::setRadius(double radius) {
    m_radius = radius;
}

bool FilletOp::execute(Document& doc) {
    if (m_bodyId < 0 || m_edges.empty() || m_radius <= 0.0) {
        return false;
    }

    try {
        // Store previous shape for undo
        m_previousShape = doc.getBody(m_bodyId);

        // Create fillet on the body shape
        BRepFilletAPI_MakeFillet fillet(m_previousShape);

        for (const auto& edge : m_edges) {
            fillet.Add(m_radius, edge);
        }

        fillet.Build();
        if (!fillet.IsDone()) {
            return false;
        }

        // Update the body with the filleted shape
        doc.updateBody(m_bodyId, fillet.Shape());
        return true;
    } catch (...) {
        return false;
    }
}

bool FilletOp::undo(Document& doc) {
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

std::string FilletOp::description() const {
    return "Fillet R" + std::to_string(m_radius) + " on " +
           std::to_string(m_edges.size()) + " edge(s)";
}

void FilletOp::renderProperties() {
    ImGui::Text("Fillet");
    ImGui::Separator();

    ImGui::InputDouble("Radius", &m_radius, 0.1, 1.0, "%.3f");

    ImGui::Text("Edges: %d selected", static_cast<int>(m_edges.size()));
    ImGui::Text("Body ID: %d", m_bodyId);
}
