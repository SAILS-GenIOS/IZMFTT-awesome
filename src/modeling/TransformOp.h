#pragma once
#include "../core/Operation.h"
#include "../core/Document.h"
#include <TopoDS_Shape.hxx>
#include <string>

enum class TransformType { Translate, Rotate, Scale };

class TransformOp : public Operation {
public:
    TransformOp();
    ~TransformOp() override = default;

    // Parameters
    void setBodyId(int id);
    void setType(TransformType type);
    void setTranslation(double dx, double dy, double dz);
    void setRotation(double ax, double ay, double az, double angleDeg);
    void setScale(double factor);

    // Getters
    int getBodyId() const { return m_bodyId; }
    TransformType getType() const { return m_type; }

    // Operation interface
    bool execute(Document& doc) override;
    bool undo(Document& doc) override;
    std::string name() const override { return "Transform"; }
    std::string description() const override;
    void renderProperties() override;
    std::string typeId() const override { return "transform"; }

private:
    int m_bodyId = -1;
    TransformType m_type = TransformType::Translate;
    double m_dx = 0, m_dy = 0, m_dz = 0;
    double m_ax = 0, m_ay = 1, m_az = 0, m_angle = 0;
    double m_scale = 1.0;
    TopoDS_Shape m_previousShape;
};
