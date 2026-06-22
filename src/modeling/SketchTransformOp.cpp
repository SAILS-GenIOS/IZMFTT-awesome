#include "SketchTransformOp.h"
#include "../core/Document.h"
#include "Sketch.h"
#include <gp_Vec.hxx>
#include <cmath>
#include <cstdio>

namespace materializr {

bool SketchTransformOp::execute(Document& doc) {
    if (m_sketchId < 0) return false;
    auto sk = doc.getSketch(m_sketchId);
    if (!sk) return false;

    if (!m_haveBefore) {
        m_planeBefore = sk->getPlane();
        m_haveBefore = true;
    }
    gp_Pln next = m_planeBefore;
    next.Transform(m_transform);
    sk->setPlane(next);

    // Apply the bundled link-state change (de-link / re-link), capturing the
    // prior state once so undo restores it.
    if (m_detachMode >= 0) {
        if (!m_haveDetachBefore) {
            m_detachedBefore = sk->isDetachedFromBody();
            m_haveDetachBefore = true;
        }
        sk->setDetachedFromBody(m_detachMode == 1);
    }

    // Build a short human-readable description for the history panel.
    gp_XYZ t = m_transform.TranslationPart();
    if (t.Modulus() > 1e-6) {
        char buf[80];
        std::snprintf(buf, sizeof(buf), "Move sketch (%.2f, %.2f, %.2f)",
                      t.X(), t.Y(), t.Z());
        m_description = buf;
    } else {
        m_description = "Rotate sketch";
    }
    return true;
}

bool SketchTransformOp::undo(Document& doc) {
    if (!m_haveBefore || m_sketchId < 0) return false;
    auto sk = doc.getSketch(m_sketchId);
    if (!sk) return false;
    sk->setPlane(m_planeBefore);
    if (m_detachMode >= 0 && m_haveDetachBefore)
        sk->setDetachedFromBody(m_detachedBefore);
    return true;
}

} // namespace materializr
