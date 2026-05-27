#pragma once
#include "../core/Operation.h"
#include "../core/Document.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <string>

class OffsetFaceOp : public Operation {
public:
    OffsetFaceOp();
    ~OffsetFaceOp() override = default;

    // Parameters
    void setBody(int id);
    void setFace(const TopoDS_Face& face);
    void setDistance(double d);

    // Getters
    int getBodyId() const { return m_bodyId; }
    double getDistance() const { return m_distance; }

    // Operation interface
    bool execute(Document& doc) override;
    bool undo(Document& doc) override;
    std::string name() const override { return "Offset Face"; }
    std::string description() const override;
    void renderProperties() override;
    std::string typeId() const override { return "offset_face"; }

private:
    int m_bodyId = -1;
    TopoDS_Face m_face;
    double m_distance = 1.0;
    TopoDS_Shape m_previousShape;
};
