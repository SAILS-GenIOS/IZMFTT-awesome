#pragma once
#include "../core/Operation.h"
#include "../core/Document.h"
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <string>

enum class PlaneCreationType {
    XY, XZ, YZ,           // standard planes
    OffsetFromPlane,       // parallel at distance
    ThroughThreePoints,    // defined by 3 points
    ParallelToFace         // parallel to a face, through a point
};

class ConstructionPlaneOp : public Operation {
public:
    ConstructionPlaneOp();
    ~ConstructionPlaneOp() override = default;

    void setType(PlaneCreationType type);
    void setOffset(double distance);
    void setBasePlane(const gp_Pln& plane);
    void setPoints(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3);
    void setName(const std::string& name);

    bool execute(Document& doc) override;
    bool undo(Document& doc) override;
    std::string name() const override { return "Construction Plane"; }
    std::string description() const override;
    void renderProperties() override;
    std::string typeId() const override { return "construction_plane"; }

private:
    gp_Pln computePlane() const;

    PlaneCreationType m_type = PlaneCreationType::XY;
    gp_Pln m_basePlane;
    double m_offset = 10.0;
    gp_Pnt m_p1, m_p2, m_p3;
    std::string m_planeName = "Plane";
    int m_createdPlaneId = -1;
};
