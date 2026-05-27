#pragma once
#include <glm/glm.hpp>
#include <TopoDS_Shape.hxx>
#include <string>
#include <vector>

class Document;
class SelectionManager;

namespace materializr {

struct MeasureResult {
    enum Type { None, Distance, EdgeLength, FaceArea, Angle, BoundingBox };
    Type type = None;
    double value = 0.0;
    std::string label;
    glm::vec3 pointA{0};
    glm::vec3 pointB{0};
    // Extra values for bounding box (width, height, depth)
    double dimX = 0.0;
    double dimY = 0.0;
    double dimZ = 0.0;
};

class MeasureTool {
public:
    MeasureTool();

    void setDocument(const Document* doc);
    void setSelectionManager(const SelectionManager* sel);

    // Update measurements based on current selection
    void update();

    // Render the measurement results panel
    void renderPanel();

    // Get results for viewport overlay rendering
    const std::vector<MeasureResult>& getResults() const;

    void clear();
    bool isActive() const;
    void setActive(bool active);

private:
    const Document* m_document = nullptr;
    const SelectionManager* m_selection = nullptr;
    std::vector<MeasureResult> m_results;
    bool m_active = false;

    void measureBodyBounds(int bodyId);
    void measureEdgeLength(const TopoDS_Shape& edge);
    void measureFaceArea(const TopoDS_Shape& face);
    void measureDistance(const TopoDS_Shape& a, const TopoDS_Shape& b);
};

} // namespace materializr
