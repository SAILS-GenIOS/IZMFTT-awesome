#pragma once

#include <glm/glm.hpp>
#include <TopoDS_Shape.hxx>
#include <optional>

class Document;
class SelectionManager;

namespace materializr {

class Camera;

struct PickResult {
    bool hit = false;
    int bodyId = -1;
    int faceIndex = -1;
    TopoDS_Shape pickedShape;    // the picked face
    TopoDS_Shape nearestEdge;    // closest edge to hit point (if any)
    float edgeScreenDist = 1e6f; // screen distance to nearest edge in pixels
    glm::vec3 hitPoint{0};
    float distance = 0;
};

class Picker {
public:
    Picker();

    // Cast a ray from screen coordinates and find the nearest hit
    PickResult pick(float screenX, float screenY,
                    float viewportWidth, float viewportHeight,
                    const Camera& camera, const Document& doc);

private:
    // Unproject screen point to world ray
    void screenToRay(float sx, float sy, float vpW, float vpH,
                     const Camera& camera,
                     glm::vec3& rayOrigin, glm::vec3& rayDir);

    // Test ray against an OCCT shape's bounding box
    bool rayIntersectsBBox(const glm::vec3& origin, const glm::vec3& dir,
                           const TopoDS_Shape& shape, float& tMin);

    int findNearestFace(const glm::vec3& origin, const glm::vec3& dir,
                        const TopoDS_Shape& shape, float& bestDist,
                        glm::vec3& hitPt, TopoDS_Shape& hitFace);

    // Find the nearest edge to a world-space point, return screen distance
    void findNearestEdge(const TopoDS_Shape& shape, const glm::vec3& hitPt,
                         float screenX, float screenY, float vpW, float vpH,
                         const Camera& camera,
                         TopoDS_Shape& nearestEdge, float& screenDist);
};

} // namespace materializr
