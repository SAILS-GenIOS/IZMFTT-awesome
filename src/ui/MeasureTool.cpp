#include "MeasureTool.h"
#include "../core/Document.h"
#include "../core/SelectionManager.h"

#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopoDS.hxx>

#include <imgui.h>
#include <cmath>
#include <cstdio>

namespace materializr {

MeasureTool::MeasureTool() = default;

void MeasureTool::setDocument(const Document* doc) {
    m_document = doc;
}

void MeasureTool::setSelectionManager(const SelectionManager* sel) {
    m_selection = sel;
}

void MeasureTool::update() {
    m_results.clear();

    if (!m_active || !m_document || !m_selection || !m_selection->hasSelection()) {
        return;
    }

    const auto& sel = m_selection->getSelection();

    if (sel.size() == 1) {
        const auto& entry = sel[0];

        switch (entry.type) {
            case SelectionType::Body:
                if (entry.bodyId >= 0) {
                    measureBodyBounds(entry.bodyId);
                }
                break;

            case SelectionType::Edge:
                if (!entry.shape.IsNull()) {
                    measureEdgeLength(entry.shape);
                }
                break;

            case SelectionType::Face:
                if (!entry.shape.IsNull()) {
                    measureFaceArea(entry.shape);
                }
                break;

            default:
                break;
        }
    } else if (sel.size() == 2) {
        // Measure distance between two selected items
        const auto& a = sel[0];
        const auto& b = sel[1];

        TopoDS_Shape shapeA, shapeB;

        // Resolve the shape for each selection entry
        if (a.type == SelectionType::Body && a.bodyId >= 0) {
            try {
                shapeA = m_document->getBody(a.bodyId);
            } catch (...) {}
        } else if (!a.shape.IsNull()) {
            shapeA = a.shape;
        }

        if (b.type == SelectionType::Body && b.bodyId >= 0) {
            try {
                shapeB = m_document->getBody(b.bodyId);
            } catch (...) {}
        } else if (!b.shape.IsNull()) {
            shapeB = b.shape;
        }

        if (!shapeA.IsNull() && !shapeB.IsNull()) {
            measureDistance(shapeA, shapeB);
        }
    }
}

void MeasureTool::measureBodyBounds(int bodyId) {
    try {
        const TopoDS_Shape& shape = m_document->getBody(bodyId);
        if (shape.IsNull()) return;

        Bnd_Box box;
        BRepBndLib::Add(shape, box);

        if (box.IsVoid()) return;

        double xMin, yMin, zMin, xMax, yMax, zMax;
        box.Get(xMin, yMin, zMin, xMax, yMax, zMax);

        double width  = xMax - xMin;
        double height = yMax - yMin;
        double depth  = zMax - zMin;

        MeasureResult result;
        result.type = MeasureResult::BoundingBox;
        result.label = "Bounding Box";
        result.dimX = width;
        result.dimY = height;
        result.dimZ = depth;
        result.pointA = glm::vec3(static_cast<float>(xMin),
                                   static_cast<float>(yMin),
                                   static_cast<float>(zMin));
        result.pointB = glm::vec3(static_cast<float>(xMax),
                                   static_cast<float>(yMax),
                                   static_cast<float>(zMax));

        m_results.push_back(result);
    } catch (...) {
        // Body not found or other error
    }
}

void MeasureTool::measureEdgeLength(const TopoDS_Shape& edge) {
    try {
        BRepAdaptor_Curve curve(TopoDS::Edge(edge));
        double length = GCPnts_AbscissaPoint::Length(curve);

        // Get start and end points of the curve
        gp_Pnt startPt = curve.Value(curve.FirstParameter());
        gp_Pnt endPt   = curve.Value(curve.LastParameter());

        MeasureResult result;
        result.type = MeasureResult::EdgeLength;
        result.value = length;
        result.label = "Edge Length";
        result.pointA = glm::vec3(static_cast<float>(startPt.X()),
                                   static_cast<float>(startPt.Y()),
                                   static_cast<float>(startPt.Z()));
        result.pointB = glm::vec3(static_cast<float>(endPt.X()),
                                   static_cast<float>(endPt.Y()),
                                   static_cast<float>(endPt.Z()));

        m_results.push_back(result);
    } catch (...) {
        // Invalid edge
    }
}

void MeasureTool::measureFaceArea(const TopoDS_Shape& face) {
    try {
        GProp_GProps props;
        BRepGProp::SurfaceProperties(face, props);
        double area = props.Mass();

        // Get the center of mass of the face
        gp_Pnt center = props.CentreOfMass();

        MeasureResult result;
        result.type = MeasureResult::FaceArea;
        result.value = area;
        result.label = "Face Area";
        result.pointA = glm::vec3(static_cast<float>(center.X()),
                                   static_cast<float>(center.Y()),
                                   static_cast<float>(center.Z()));

        m_results.push_back(result);
    } catch (...) {
        // Invalid face
    }
}

void MeasureTool::measureDistance(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    try {
        BRepExtrema_DistShapeShape distCalc(a, b);
        if (!distCalc.IsDone() || distCalc.NbSolution() == 0) {
            return;
        }

        double dist = distCalc.Value();

        gp_Pnt ptA = distCalc.PointOnShape1(1);
        gp_Pnt ptB = distCalc.PointOnShape2(1);

        MeasureResult result;
        result.type = MeasureResult::Distance;
        result.value = dist;
        result.label = "Distance";
        result.pointA = glm::vec3(static_cast<float>(ptA.X()),
                                   static_cast<float>(ptA.Y()),
                                   static_cast<float>(ptA.Z()));
        result.pointB = glm::vec3(static_cast<float>(ptB.X()),
                                   static_cast<float>(ptB.Y()),
                                   static_cast<float>(ptB.Z()));

        m_results.push_back(result);
    } catch (...) {
        // Distance computation failed
    }
}

void MeasureTool::renderPanel() {
    if (!m_active) return;

    ImGui::Begin("Measure", &m_active);

    if (!m_selection || !m_selection->hasSelection()) {
        ImGui::TextWrapped("Select geometry to measure.\n\n"
                           "- 1 body: bounding box dimensions\n"
                           "- 1 edge: curve length\n"
                           "- 1 face: surface area\n"
                           "- 2 items: minimum distance");
    }

    if (m_results.empty() && m_selection && m_selection->hasSelection()) {
        ImGui::Text("No measurements available for this selection.");
    }

    for (const auto& r : m_results) {
        ImGui::Separator();

        switch (r.type) {
            case MeasureResult::Distance:
                ImGui::Text("Distance: %.2f mm", r.value);
                ImGui::Text("  From: (%.2f, %.2f, %.2f)",
                            r.pointA.x, r.pointA.y, r.pointA.z);
                ImGui::Text("  To:   (%.2f, %.2f, %.2f)",
                            r.pointB.x, r.pointB.y, r.pointB.z);
                break;

            case MeasureResult::EdgeLength:
                ImGui::Text("Length: %.2f mm", r.value);
                ImGui::Text("  Start: (%.2f, %.2f, %.2f)",
                            r.pointA.x, r.pointA.y, r.pointA.z);
                ImGui::Text("  End:   (%.2f, %.2f, %.2f)",
                            r.pointB.x, r.pointB.y, r.pointB.z);
                break;

            case MeasureResult::FaceArea:
                ImGui::Text("Area: %.2f mm%s", r.value, "\xC2\xB2");
                ImGui::Text("  Center: (%.2f, %.2f, %.2f)",
                            r.pointA.x, r.pointA.y, r.pointA.z);
                break;

            case MeasureResult::BoundingBox:
                ImGui::Text("Bounding Box:");
                ImGui::Text("  Width (X):  %.2f mm", r.dimX);
                ImGui::Text("  Height (Y): %.2f mm", r.dimY);
                ImGui::Text("  Depth (Z):  %.2f mm", r.dimZ);
                ImGui::Text("  Min: (%.2f, %.2f, %.2f)",
                            r.pointA.x, r.pointA.y, r.pointA.z);
                ImGui::Text("  Max: (%.2f, %.2f, %.2f)",
                            r.pointB.x, r.pointB.y, r.pointB.z);
                break;

            case MeasureResult::Angle:
                ImGui::Text("Angle: %.2f deg", r.value);
                break;

            default:
                break;
        }
    }

    ImGui::End();
}

const std::vector<MeasureResult>& MeasureTool::getResults() const {
    return m_results;
}

void MeasureTool::clear() {
    m_results.clear();
}

bool MeasureTool::isActive() const {
    return m_active;
}

void MeasureTool::setActive(bool active) {
    m_active = active;
    if (!active) {
        m_results.clear();
    }
}

} // namespace materializr
