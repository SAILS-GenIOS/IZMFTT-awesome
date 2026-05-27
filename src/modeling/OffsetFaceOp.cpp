#include "OffsetFaceOp.h"
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <imgui.h>

OffsetFaceOp::OffsetFaceOp() = default;

void OffsetFaceOp::setBody(int id) {
    m_bodyId = id;
}

void OffsetFaceOp::setFace(const TopoDS_Face& face) {
    m_face = face;
}

void OffsetFaceOp::setDistance(double d) {
    m_distance = d;
}

bool OffsetFaceOp::execute(Document& doc) {
    if (m_bodyId < 0 || m_face.IsNull()) {
        return false;
    }

    try {
        // Store previous shape for undo
        m_previousShape = doc.getBody(m_bodyId);

        // Use MakeThickSolid with the target face to push/pull it by the
        // given distance. Passing the face in the "closing faces" list and
        // a positive offset effectively moves that face outward.
        TopTools_ListOfShape facesToOffset;
        facesToOffset.Append(m_face);

        BRepOffsetAPI_MakeThickSolid offsetMaker;
        offsetMaker.MakeThickSolidByJoin(
            m_previousShape,
            facesToOffset,
            m_distance,   // positive = outward, negative = inward
            1.0e-3        // tolerance
        );
        offsetMaker.Build();
        if (!offsetMaker.IsDone()) {
            return false;
        }

        doc.updateBody(m_bodyId, offsetMaker.Shape());
        return true;
    } catch (...) {
        return false;
    }
}

bool OffsetFaceOp::undo(Document& doc) {
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

std::string OffsetFaceOp::description() const {
    return "Offset Face by " + std::to_string(m_distance);
}

void OffsetFaceOp::renderProperties() {
    ImGui::Text("Offset Face");
    ImGui::Separator();

    ImGui::InputDouble("Distance", &m_distance, 0.1, 1.0, "%.3f");

    ImGui::Text("Body ID: %d", m_bodyId);
    ImGui::Text("Face: %s", m_face.IsNull() ? "none" : "selected");
}
