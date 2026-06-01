#pragma once
#include "../core/Operation.h"
#include <gp_Trsf.hxx>
#include <gp_Pln.hxx>
#include <string>

namespace materializr {

// Lightweight transform op for a single sketch's plane. Used when the user
// drags the Move/Rotate gizmo on a selected standalone sketch (one with no
// source body — the body-attached case still routes through TransformOp's
// sketch-propagation path). Stores the before-plane explicitly for undo so
// it works even if m_transform itself isn't perfectly invertible numerically.
class SketchTransformOp : public Operation {
public:
    SketchTransformOp() = default;
    ~SketchTransformOp() override = default;

    void setSketch(int id) { m_sketchId = id; }
    void setTransform(const gp_Trsf& t) { m_transform = t; }

    bool execute(Document& doc) override;
    bool undo(Document& doc) override;

    std::string name() const override { return "Sketch Transform"; }
    std::string description() const override { return m_description; }
    void renderProperties() override {}
    std::string typeId() const override { return "sketchtransform"; }

private:
    int m_sketchId = -1;
    gp_Trsf m_transform;     // applied to the sketch's plane on execute
    gp_Pln  m_planeBefore;   // captured on first execute, restored on undo
    bool m_haveBefore = false;
    std::string m_description = "Sketch transform";
};

} // namespace materializr
