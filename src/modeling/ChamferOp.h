#pragma once
#include "../core/Operation.h"
#include "../core/Document.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <vector>
#include <string>

class ChamferOp : public Operation {
public:
    ChamferOp();
    ~ChamferOp() override = default;

    // Parameters
    void setBody(int bodyId);
    void setEdges(const std::vector<TopoDS_Edge>& edges);
    void setDistance(double distance);

    // Getters
    int getBodyId() const { return m_bodyId; }
    double getDistance() const { return m_distance; }
    const std::vector<TopoDS_Edge>& getEdges() const { return m_edges; }

    // Operation interface
    bool execute(Document& doc) override;
    bool undo(Document& doc) override;
    std::string name() const override { return "Chamfer"; }
    std::string description() const override;
    void renderProperties() override;
    std::string typeId() const override { return "chamfer"; }

private:
    int m_bodyId = -1;
    std::vector<TopoDS_Edge> m_edges;
    double m_distance = 1.0;
    TopoDS_Shape m_previousShape; // for undo
};
