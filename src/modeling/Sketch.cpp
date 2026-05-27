#include "Sketch.h"

#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepGProp.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <Geom_Curve.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <ElCLib.hxx>
#include <gp_Pnt.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt2d.hxx>
#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cmath>

namespace materializr {

Sketch::Sketch()
    : m_plane(gp_Pln()) // default XY plane at origin
{
}

void Sketch::setPlane(const gp_Pln& plane) {
    m_plane = plane;
}

const gp_Pln& Sketch::getPlane() const {
    return m_plane;
}

gp_Pnt Sketch::sketchToWorld(glm::vec2 pt2d) const {
    const gp_Ax3& ax = m_plane.Position();
    gp_Pnt origin = ax.Location();
    gp_Dir xDir = ax.XDirection();
    gp_Dir yDir = ax.YDirection();
    return gp_Pnt(
        origin.X() + pt2d.x * xDir.X() + pt2d.y * yDir.X(),
        origin.Y() + pt2d.x * xDir.Y() + pt2d.y * yDir.Y(),
        origin.Z() + pt2d.x * xDir.Z() + pt2d.y * yDir.Z()
    );
}

// Point management

int Sketch::addPoint(glm::vec2 pos) {
    SketchPoint pt;
    pt.id = nextId();
    pt.pos = pos;
    pt.isConstruction = false;
    m_points.push_back(pt);
    return pt.id;
}

void Sketch::movePoint(int id, glm::vec2 pos) {
    SketchPoint* pt = findPoint(id);
    if (pt) {
        pt->pos = pos;
    }
}

const SketchPoint* Sketch::getPoint(int id) const {
    for (const auto& pt : m_points) {
        if (pt.id == id) return &pt;
    }
    return nullptr;
}

const std::vector<SketchPoint>& Sketch::getPoints() const {
    return m_points;
}

SketchPoint* Sketch::findPoint(int id) {
    for (auto& pt : m_points) {
        if (pt.id == id) return &pt;
    }
    return nullptr;
}

// Element creation

int Sketch::addLine(int startPtId, int endPtId) {
    SketchLine line;
    line.id = nextId();
    line.startPointId = startPtId;
    line.endPointId = endPtId;
    line.isConstruction = false;
    m_lines.push_back(line);
    return line.id;
}

int Sketch::addCircle(int centerPtId, double radius) {
    SketchCircle circle;
    circle.id = nextId();
    circle.centerPointId = centerPtId;
    circle.radius = radius;
    circle.isConstruction = false;
    m_circles.push_back(circle);
    return circle.id;
}

int Sketch::addArc(int centerPtId, int startPtId, int endPtId, double radius) {
    SketchArc arc;
    arc.id = nextId();
    arc.centerPointId = centerPtId;
    arc.startPointId = startPtId;
    arc.endPointId = endPtId;
    arc.radius = radius;
    arc.isConstruction = false;
    m_arcs.push_back(arc);
    return arc.id;
}

int Sketch::addSpline(const std::vector<int>& controlPointIds) {
    SketchSpline spline;
    spline.id = nextId();
    spline.controlPointIds = controlPointIds;
    spline.isConstruction = false;
    m_splines.push_back(spline);
    return spline.id;
}

int Sketch::addPolygon(int centerPtId, double radius, int sides) {
    SketchPolygon polygon;
    polygon.id = nextId();
    polygon.centerPointId = centerPtId;
    polygon.radius = radius;
    polygon.sides = sides;

    const SketchPoint* center = getPoint(centerPtId);
    if (!center) return -1;

    // Create N vertex points evenly spaced around center
    for (int i = 0; i < sides; ++i) {
        double angle = 2.0 * M_PI * i / sides;
        float vx = center->pos.x + static_cast<float>(radius * std::cos(angle));
        float vy = center->pos.y + static_cast<float>(radius * std::sin(angle));
        int ptId = addPoint(glm::vec2(vx, vy));
        polygon.vertexPointIds.push_back(ptId);
    }

    // Create N lines connecting consecutive vertices (closing the loop)
    for (int i = 0; i < sides; ++i) {
        int startPtId = polygon.vertexPointIds[i];
        int endPtId = polygon.vertexPointIds[(i + 1) % sides];
        int lineId = addLine(startPtId, endPtId);
        polygon.lineIds.push_back(lineId);
    }

    polygon.isConstruction = false;
    m_polygons.push_back(polygon);
    return polygon.id;
}

int Sketch::addRectangle(glm::vec2 corner1, glm::vec2 corner2) {
    // Create 4 corner points
    glm::vec2 c1 = corner1;
    glm::vec2 c2 = glm::vec2(corner2.x, corner1.y);
    glm::vec2 c3 = corner2;
    glm::vec2 c4 = glm::vec2(corner1.x, corner2.y);

    int p1 = addPoint(c1);
    int p2 = addPoint(c2);
    int p3 = addPoint(c3);
    int p4 = addPoint(c4);

    // Create 4 lines forming a closed rectangle
    int firstLineId = addLine(p1, p2);
    addLine(p2, p3);
    addLine(p3, p4);
    addLine(p4, p1);

    return firstLineId;
}

// Element access

const std::vector<SketchLine>& Sketch::getLines() const {
    return m_lines;
}

const std::vector<SketchCircle>& Sketch::getCircles() const {
    return m_circles;
}

const std::vector<SketchArc>& Sketch::getArcs() const {
    return m_arcs;
}

const std::vector<SketchSpline>& Sketch::getSplines() const {
    return m_splines;
}

const std::vector<SketchPolygon>& Sketch::getPolygons() const {
    return m_polygons;
}

// Element removal

void Sketch::removeElement(int id) {
    m_lines.erase(
        std::remove_if(m_lines.begin(), m_lines.end(),
            [id](const SketchLine& l) { return l.id == id; }),
        m_lines.end());

    m_circles.erase(
        std::remove_if(m_circles.begin(), m_circles.end(),
            [id](const SketchCircle& c) { return c.id == id; }),
        m_circles.end());

    m_arcs.erase(
        std::remove_if(m_arcs.begin(), m_arcs.end(),
            [id](const SketchArc& a) { return a.id == id; }),
        m_arcs.end());

    m_splines.erase(
        std::remove_if(m_splines.begin(), m_splines.end(),
            [id](const SketchSpline& s) { return s.id == id; }),
        m_splines.end());

    m_polygons.erase(
        std::remove_if(m_polygons.begin(), m_polygons.end(),
            [id](const SketchPolygon& p) { return p.id == id; }),
        m_polygons.end());

    m_points.erase(
        std::remove_if(m_points.begin(), m_points.end(),
            [id](const SketchPoint& p) { return p.id == id; }),
        m_points.end());
}

void Sketch::clear() {
    m_points.clear();
    m_lines.clear();
    m_circles.clear();
    m_arcs.clear();
    m_splines.clear();
    m_polygons.clear();
    m_nextId = 1;
}

// Build OCCT wires from closed profiles.
//
// Adjacency graph spans lines AND arc-segments-of-circles. A circle that has no
// sketch points on its perimeter contributes a standalone full-circle wire. A
// circle that *does* have points on it gets split into arc segments at those
// points so the DFS can find closed loops mixing straight and curved edges.

std::vector<TopoDS_Wire> Sketch::buildWires() const {
    std::vector<TopoDS_Wire> wires;

    // Helper edge spec used by the adjacency walker
    struct EdgeSpec {
        bool isArc = false;
        int startPtId = -1;
        int endPtId = -1;
        // Arc-only fields:
        int circleId = -1;           // index into m_circles
        float startAngle = 0.0f;     // radians, around circle center
        float endAngle = 0.0f;       // radians, CCW from startAngle (may exceed 2pi)
    };
    std::vector<EdgeSpec> edges;

    // Add line edges. If any sketch point lies on the interior of a line (e.g.
    // because trim split the line geometrically but not topologically), emit
    // sub-edges so adjacency can route through that point. Without this, the
    // line stays one edge from corner to corner and any element ending mid-line
    // can never form a closed region.
    const float onLineTol = 1e-2f;
    for (const auto& line : m_lines) {
        const SketchPoint* a = getPoint(line.startPointId);
        const SketchPoint* b = getPoint(line.endPointId);
        if (!a || !b) {
            edges.push_back({false, line.startPointId, line.endPointId, -1, 0, 0});
            continue;
        }
        glm::vec2 dir = b->pos - a->pos;
        float len2 = glm::dot(dir, dir);
        if (len2 < 1e-12f) {
            edges.push_back({false, line.startPointId, line.endPointId, -1, 0, 0});
            continue;
        }

        struct Onseg { float t; int id; };
        std::vector<Onseg> onseg;
        onseg.push_back({0.0f, line.startPointId});
        onseg.push_back({1.0f, line.endPointId});
        for (const auto& pt : m_points) {
            if (pt.id == line.startPointId || pt.id == line.endPointId) continue;
            glm::vec2 v = pt.pos - a->pos;
            float t = glm::dot(v, dir) / len2;
            if (t < 1e-3f || t > 1.0f - 1e-3f) continue;
            glm::vec2 proj = a->pos + t * dir;
            if (glm::length(pt.pos - proj) > onLineTol) continue;
            onseg.push_back({t, pt.id});
        }
        std::sort(onseg.begin(), onseg.end(),
                  [](const Onseg& x, const Onseg& y) { return x.t < y.t; });
        for (size_t i = 0; i + 1 < onseg.size(); ++i) {
            edges.push_back({false, onseg[i].id, onseg[i + 1].id, -1, 0, 0});
        }
    }

    // For each circle, check for points lying on its perimeter
    const float perimeterEpsilon = 1e-3f;
    for (size_t ci = 0; ci < m_circles.size(); ++ci) {
        const auto& circle = m_circles[ci];
        const SketchPoint* center = getPoint(circle.centerPointId);
        if (!center) continue;

        struct OnPerim { int ptId; float angle; };
        std::vector<OnPerim> onPerim;
        for (const auto& pt : m_points) {
            if (pt.id == circle.centerPointId) continue;
            float d = glm::length(pt.pos - center->pos);
            if (std::abs(d - static_cast<float>(circle.radius)) < perimeterEpsilon) {
                float a = std::atan2(pt.pos.y - center->pos.y, pt.pos.x - center->pos.x);
                if (a < 0.0f) a += 2.0f * static_cast<float>(M_PI);
                onPerim.push_back({pt.id, a});
            }
        }

        if (onPerim.empty()) {
            // Standalone circle: emit a full-circle wire directly
            gp_Pnt center3d = sketchToWorld(center->pos);
            gp_Dir normal = m_plane.Position().Direction();
            gp_Circ gpCircle(gp_Ax2(center3d, normal), circle.radius);
            BRepBuilderAPI_MakeEdge edgeMaker(gpCircle);
            if (edgeMaker.IsDone()) {
                BRepBuilderAPI_MakeWire wireMaker(edgeMaker.Edge());
                if (wireMaker.IsDone()) wires.push_back(wireMaker.Wire());
            }
            continue;
        }

        // Split the circle into arc segments at perimeter points (CCW)
        std::sort(onPerim.begin(), onPerim.end(),
                  [](const OnPerim& a, const OnPerim& b){ return a.angle < b.angle; });
        for (size_t i = 0; i < onPerim.size(); ++i) {
            const OnPerim& cur = onPerim[i];
            const OnPerim& nxt = onPerim[(i + 1) % onPerim.size()];
            float endAng = nxt.angle;
            if (endAng <= cur.angle) endAng += 2.0f * static_cast<float>(M_PI);
            edges.push_back({true, cur.ptId, nxt.ptId, static_cast<int>(ci), cur.angle, endAng});
        }
    }

    // Existing arcs: include them in adjacency too. Same intermediate-point
    // split as for lines — any sketch point lying on the arc's perimeter
    // inside the arc's angle range becomes a virtual split, so adjacency can
    // route through it.
    for (const auto& arc : m_arcs) {
        const SketchPoint* center = getPoint(arc.centerPointId);
        const SketchPoint* s = getPoint(arc.startPointId);
        const SketchPoint* e = getPoint(arc.endPointId);
        if (!center || !s || !e) continue;
        float sA = std::atan2(s->pos.y - center->pos.y, s->pos.x - center->pos.x);
        float eA = std::atan2(e->pos.y - center->pos.y, e->pos.x - center->pos.x);
        if (sA < 0.0f) sA += 2.0f * static_cast<float>(M_PI);
        if (eA < 0.0f) eA += 2.0f * static_cast<float>(M_PI);
        if (eA <= sA) eA += 2.0f * static_cast<float>(M_PI);
        int arcEncoded = -2 - static_cast<int>(&arc - &m_arcs[0]);

        struct Onarc { float angle; int id; };
        std::vector<Onarc> onarc;
        onarc.push_back({sA, arc.startPointId});
        onarc.push_back({eA, arc.endPointId});
        for (const auto& pt : m_points) {
            if (pt.id == arc.startPointId || pt.id == arc.endPointId ||
                pt.id == arc.centerPointId) continue;
            float d = glm::length(pt.pos - center->pos);
            if (std::abs(d - static_cast<float>(arc.radius)) > perimeterEpsilon) continue;
            float a = std::atan2(pt.pos.y - center->pos.y, pt.pos.x - center->pos.x);
            if (a < 0.0f) a += 2.0f * static_cast<float>(M_PI);
            // Bring `a` into the [sA, sA + 2π) range so it can be compared against eA.
            while (a < sA - 1e-4f) a += 2.0f * static_cast<float>(M_PI);
            if (a < sA + 1e-3f || a > eA - 1e-3f) continue;
            onarc.push_back({a, pt.id});
        }
        std::sort(onarc.begin(), onarc.end(),
                  [](const Onarc& x, const Onarc& y) { return x.angle < y.angle; });
        for (size_t i = 0; i + 1 < onarc.size(); ++i) {
            edges.push_back({true, onarc[i].id, onarc[i + 1].id, arcEncoded,
                              onarc[i].angle, onarc[i + 1].angle});
        }
    }

    if (edges.empty()) return wires;

    // Adjacency: pointId -> list of edge indices
    std::unordered_map<int, std::vector<int>> pointToEdges;
    for (size_t i = 0; i < edges.size(); ++i) {
        pointToEdges[edges[i].startPtId].push_back(static_cast<int>(i));
        pointToEdges[edges[i].endPtId].push_back(static_cast<int>(i));
    }

    std::unordered_set<int> usedEdges;

    auto otherEnd = [&](const EdgeSpec& e, int p) {
        return (e.startPtId == p) ? e.endPtId : e.startPtId;
    };

    auto emitOcctEdge = [&](const EdgeSpec& es, int fromPt, BRepBuilderAPI_MakeWire& wm) -> bool {
        const SketchPoint* sp = getPoint(es.startPtId);
        const SketchPoint* ep = getPoint(es.endPtId);
        if (!sp || !ep) return false;
        gp_Pnt p1 = sketchToWorld(sp->pos);
        gp_Pnt p2 = sketchToWorld(ep->pos);
        if (p1.Distance(p2) < 1e-10 && !es.isArc) return false;

        if (!es.isArc) {
            BRepBuilderAPI_MakeEdge mk(p1, p2);
            if (!mk.IsDone()) return false;
            wm.Add(mk.Edge());
            return true;
        }

        // Arc segment: figure out center/radius from either a circle index or an arc index
        glm::vec2 center2d(0); double radius = 0.0;
        if (es.circleId >= 0 && es.circleId < static_cast<int>(m_circles.size())) {
            const SketchCircle& c = m_circles[es.circleId];
            const SketchPoint* cp = getPoint(c.centerPointId);
            if (!cp) return false;
            center2d = cp->pos;
            radius = c.radius;
        } else {
            int arcIdx = -es.circleId - 2;
            if (arcIdx < 0 || arcIdx >= static_cast<int>(m_arcs.size())) return false;
            const SketchArc& a = m_arcs[arcIdx];
            const SketchPoint* cp = getPoint(a.centerPointId);
            if (!cp) return false;
            center2d = cp->pos;
            radius = a.radius;
        }

        // Walk direction: if we're entering from startPtId go CCW (start->end angles),
        // otherwise go the other way.
        float aStart = es.startAngle;
        float aEnd = es.endAngle;
        if (fromPt == es.endPtId) std::swap(aStart, aEnd);
        float aMid = 0.5f * (aStart + aEnd);
        glm::vec2 mid2d(center2d.x + std::cos(aMid) * static_cast<float>(radius),
                        center2d.y + std::sin(aMid) * static_cast<float>(radius));
        gp_Pnt mid3d = sketchToWorld(mid2d);

        // p1 must correspond to fromPt and p2 to the other end
        gp_Pnt fromPnt = (fromPt == es.startPtId) ? p1 : p2;
        gp_Pnt toPnt   = (fromPt == es.startPtId) ? p2 : p1;

        GC_MakeArcOfCircle arcMaker(fromPnt, mid3d, toPnt);
        if (!arcMaker.IsDone()) return false;
        BRepBuilderAPI_MakeEdge mk(arcMaker.Value());
        if (!mk.IsDone()) return false;
        wm.Add(mk.Edge());
        return true;
    };

    for (size_t startIdx = 0; startIdx < edges.size(); ++startIdx) {
        if (usedEdges.count(static_cast<int>(startIdx))) continue;

        std::vector<int> chain;
        chain.push_back(static_cast<int>(startIdx));
        usedEdges.insert(static_cast<int>(startIdx));

        int firstPointId = edges[startIdx].startPtId;
        int currentPointId = edges[startIdx].endPtId;

        bool closed = false;
        bool extended = true;
        while (extended && !closed) {
            extended = false;
            if (currentPointId == firstPointId) { closed = true; break; }
            auto it = pointToEdges.find(currentPointId);
            if (it == pointToEdges.end()) break;
            for (int idx : it->second) {
                if (usedEdges.count(idx)) continue;
                int nextPt = otherEnd(edges[idx], currentPointId);
                if (nextPt < 0) continue;
                chain.push_back(idx);
                usedEdges.insert(idx);
                currentPointId = nextPt;
                extended = true;
                break;
            }
        }

        if (!closed) {
            for (int idx : chain) usedEdges.erase(idx);
            continue;
        }

        // Emit OCCT wire from the chain
        BRepBuilderAPI_MakeWire wireMaker;
        int curPt = firstPointId;
        bool valid = true;
        for (int idx : chain) {
            const EdgeSpec& es = edges[idx];
            if (!emitOcctEdge(es, curPt, wireMaker)) { valid = false; break; }
            curPt = otherEnd(es, curPt);
        }

        if (valid && wireMaker.IsDone()) wires.push_back(wireMaker.Wire());
    }

    return wires;
}

namespace {

// Sample a 2D point on a wire (used as a "representative" for containment tests).
// We pick the midpoint of the first edge in sketch-plane coordinates.
glm::vec2 sampleWirePoint(const TopoDS_Wire& wire, const gp_Pln& plane) {
    gp_Ax3 ax = plane.Position();
    gp_Pnt origin = ax.Location();
    gp_Dir xd = ax.XDirection();
    gp_Dir yd = ax.YDirection();

    auto toSketch2D = [&](const gp_Pnt& w) {
        gp_Vec v(origin, w);
        return glm::vec2(static_cast<float>(v.Dot(gp_Vec(xd))),
                         static_cast<float>(v.Dot(gp_Vec(yd))));
    };

    BRepTools_WireExplorer ex(wire);
    if (!ex.More()) return glm::vec2(0);
    TopoDS_Edge edge = TopoDS::Edge(ex.Current());
    double f, l;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
    if (curve.IsNull()) return glm::vec2(0);
    gp_Pnt mid;
    curve->D0((f + l) * 0.5, mid);
    return toSketch2D(mid);
}

// Densely sample a wire's perimeter into 2D sketch-plane points.
void densifyWire2D(const TopoDS_Wire& wire, const gp_Pln& plane,
                   std::vector<glm::vec2>& out, int samplesPerEdge = 32) {
    gp_Ax3 ax = plane.Position();
    gp_Pnt origin = ax.Location();
    gp_Dir xd = ax.XDirection();
    gp_Dir yd = ax.YDirection();
    auto toSketch2D = [&](const gp_Pnt& w) {
        gp_Vec v(origin, w);
        return glm::vec2(static_cast<float>(v.Dot(gp_Vec(xd))),
                         static_cast<float>(v.Dot(gp_Vec(yd))));
    };

    for (BRepTools_WireExplorer ex(wire); ex.More(); ex.Next()) {
        TopoDS_Edge edge = TopoDS::Edge(ex.Current());
        double f, l;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
        if (curve.IsNull()) continue;
        for (int i = 0; i < samplesPerEdge; ++i) {
            double t = f + (l - f) * (double(i) / samplesPerEdge);
            gp_Pnt p;
            curve->D0(t, p);
            out.push_back(toSketch2D(p));
        }
    }
}

// Standard ray-cast point-in-polygon (winding-independent, counts crossings).
bool pointInPolygon2D(const std::vector<glm::vec2>& poly, glm::vec2 p) {
    bool inside = false;
    size_t n = poly.size();
    if (n < 3) return false;
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const auto& a = poly[i];
        const auto& b = poly[j];
        bool intersect = ((a.y > p.y) != (b.y > p.y)) &&
            (p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y + 1e-30f) + a.x);
        if (intersect) inside = !inside;
    }
    return inside;
}

} // anonymous

std::vector<Sketch::Region> Sketch::buildRegions() const {
    std::vector<Region> regions;
    auto wires = buildWires();
    if (wires.empty()) return regions;

    // Densify each wire into a 2D polygon (sketch-plane coordinates) for containment tests.
    std::vector<std::vector<glm::vec2>> polys(wires.size());
    std::vector<glm::vec2> samplePoints(wires.size());
    for (size_t i = 0; i < wires.size(); ++i) {
        densifyWire2D(wires[i], m_plane, polys[i]);
        samplePoints[i] = sampleWirePoint(wires[i], m_plane);
    }

    // For each pair (a, b), check if a's sample point lies inside b's polygon.
    std::vector<std::vector<int>> containedBy(wires.size()); // wires whose polygon contains me
    for (size_t a = 0; a < wires.size(); ++a) {
        for (size_t b = 0; b < wires.size(); ++b) {
            if (a == b) continue;
            if (pointInPolygon2D(polys[b], samplePoints[a])) {
                containedBy[a].push_back(static_cast<int>(b));
            }
        }
    }

    // For each wire, its direct parent is the wire that contains it but isn't contained by any
    // intermediate wire. We build a containment tree, then each wire becomes one region whose
    // holes are its direct children.
    auto directParent = [&](int i) -> int {
        int best = -1;
        size_t bestDepth = 0;
        for (int candidate : containedBy[i]) {
            // depth = how many wires contain `candidate`
            size_t depth = containedBy[candidate].size();
            if (best < 0 || depth > bestDepth) {
                best = candidate;
                bestDepth = depth;
            }
        }
        return best;
    };

    std::vector<std::vector<int>> children(wires.size());
    for (size_t i = 0; i < wires.size(); ++i) {
        int p = directParent(static_cast<int>(i));
        if (p >= 0) children[p].push_back(static_cast<int>(i));
    }

    // Emit one region per wire: outer = this wire, holes = direct children
    for (size_t i = 0; i < wires.size(); ++i) {
        Region region;
        region.outerWire = wires[i];
        for (int c : children[i]) {
            // Reverse child wire so it acts as a hole
            region.holeWires.push_back(TopoDS::Wire(wires[c].Reversed()));
        }

        // Representative point: sample point of this wire, nudged toward the interior.
        // For simplicity use the centroid of the polygon (mean of samples).
        glm::vec2 mean(0);
        for (auto& q : polys[i]) mean += q;
        if (!polys[i].empty()) mean /= float(polys[i].size());
        // If mean falls inside a hole, nudge to the wire sample point.
        bool meanInHole = false;
        for (int c : children[i]) {
            if (pointInPolygon2D(polys[c], mean)) { meanInHole = true; break; }
        }
        region.representativePoint = meanInHole ? samplePoints[i] : mean;

        // Build the planar face
        try {
            BRepBuilderAPI_MakeFace mf(m_plane, region.outerWire);
            for (const auto& h : region.holeWires) mf.Add(h);
            mf.Build();
            if (mf.IsDone()) region.face = mf.Face();
        } catch (...) { /* leave face null */ }

        regions.push_back(std::move(region));
    }

    // Complement region: if this sketch was started on an existing face, emit
    // one extra region representing the source face minus all top-level sketch
    // regions. Lets users push/pull the area inside the host face but outside
    // the sketch (e.g. the square area surrounding a circle drawn on it).
    if (!m_sourceFace.IsNull()) {
        std::vector<int> topLevel;
        for (size_t i = 0; i < wires.size(); ++i) {
            if (directParent(static_cast<int>(i)) < 0) topLevel.push_back(static_cast<int>(i));
        }
        TopoDS_Wire faceOuter = BRepTools::OuterWire(m_sourceFace);
        if (!topLevel.empty() && !faceOuter.IsNull()) {
            Region complement;
            complement.outerWire = faceOuter;
            // Holes: the source face's own inner wires (existing holes in the host
            // face) plus each top-level sketch region (reversed for OCCT).
            for (TopExp_Explorer wexp(m_sourceFace, TopAbs_WIRE); wexp.More(); wexp.Next()) {
                TopoDS_Wire w = TopoDS::Wire(wexp.Current());
                if (!w.IsSame(faceOuter)) complement.holeWires.push_back(w);
            }
            for (int t : topLevel) {
                complement.holeWires.push_back(TopoDS::Wire(wires[t].Reversed()));
            }
            try {
                BRepBuilderAPI_MakeFace mf(m_plane, complement.outerWire);
                for (const auto& h : complement.holeWires) mf.Add(h);
                mf.Build();
                if (mf.IsDone()) complement.face = mf.Face();
            } catch (...) {}

            std::vector<glm::vec2> outerPoly;
            densifyWire2D(complement.outerWire, m_plane, outerPoly);
            glm::vec2 candidate(0);
            if (!outerPoly.empty()) {
                for (auto& q : outerPoly) candidate += q;
                candidate /= float(outerPoly.size());
            }
            // If the centroid happens to fall inside a top-level region, fall back
            // to a sample on the source face's outer wire (just inside the face).
            for (int t : topLevel) {
                if (pointInPolygon2D(polys[t], candidate)) {
                    candidate = sampleWirePoint(complement.outerWire, m_plane);
                    break;
                }
            }
            complement.representativePoint = candidate;

            regions.push_back(std::move(complement));
        }
    }

    return regions;
}

bool Sketch::isPointInRegion(const Region& region, glm::vec2 p) const {
    std::vector<glm::vec2> outerPoly;
    densifyWire2D(region.outerWire, m_plane, outerPoly);
    if (!pointInPolygon2D(outerPoly, p)) return false;
    for (const auto& h : region.holeWires) {
        std::vector<glm::vec2> holePoly;
        densifyWire2D(h, m_plane, holePoly);
        if (pointInPolygon2D(holePoly, p)) return false;
    }
    return true;
}

bool Sketch::getSourceFaceCentroid(glm::vec2& out) const {
    if (m_sourceFace.IsNull()) return false;
    if (m_centroidValid) { out = m_centroid; return true; }

    // Let OCCT compute the true area centroid (centre of mass for a uniform
    // density surface). Doing this ourselves from densified polygon vertices is
    // fragile for faces whose outer wire has any reversed edges — the resulting
    // vertex sequence is scrambled and the polygon-area formula returns garbage.
    try {
        GProp_GProps props;
        BRepGProp::SurfaceProperties(m_sourceFace, props);
        if (props.Mass() <= 0.0) return false;
        gp_Pnt c3d = props.CentreOfMass();
        // Project to sketch-plane 2D coordinates.
        const gp_Ax3& ax = m_plane.Position();
        gp_Pnt origin = ax.Location();
        gp_Dir xd = ax.XDirection();
        gp_Dir yd = ax.YDirection();
        gp_Vec v(origin, c3d);
        m_centroid = glm::vec2(static_cast<float>(v.Dot(gp_Vec(xd))),
                               static_cast<float>(v.Dot(gp_Vec(yd))));
        m_centroidValid = true;
        out = m_centroid;
        return true;
    } catch (...) { return false; }
}

int Sketch::elementCount() const {
    return static_cast<int>(m_lines.size() + m_circles.size() + m_arcs.size() +
                            m_splines.size() + m_polygons.size());
}

int Sketch::pointCount() const {
    return static_cast<int>(m_points.size());
}

} // namespace materializr
