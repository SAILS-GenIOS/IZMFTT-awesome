#include "SketchEditOp.h"
#include <cmath>
#include <cstdio>

namespace materializr {

SketchEditOp::SketchEditOp(std::shared_ptr<Sketch> liveSketch,
                           std::shared_ptr<Sketch> beforeSnapshot,
                           std::shared_ptr<Sketch> afterSnapshot)
    : m_target(std::move(liveSketch)),
      m_before(std::move(beforeSnapshot)),
      m_after(std::move(afterSnapshot)) {}

bool SketchEditOp::execute(Document& /*doc*/) {
    if (!m_target || !m_after) return false;
    *m_target = *m_after;
    return true;
}

bool SketchEditOp::undo(Document& /*doc*/) {
    if (!m_target || !m_before) return false;
    *m_target = *m_before;
    return true;
}

// Helper: human-friendly name for a ConstraintType. Used in descriptions.
static const char* constraintName(ConstraintType t) {
    switch (t) {
        case ConstraintType::Coincident:    return "Coincident";
        case ConstraintType::Horizontal:    return "Horizontal";
        case ConstraintType::Vertical:      return "Vertical";
        case ConstraintType::Distance:      return "Distance";
        case ConstraintType::Radius:        return "Ø";
        case ConstraintType::Parallel:      return "Parallel";
        case ConstraintType::Perpendicular: return "Perpendicular";
        case ConstraintType::Fixed:         return "Fix Position";
        case ConstraintType::Tangent:       return "Tangent";
        case ConstraintType::Equal:         return "Equal";
        case ConstraintType::Concentric:    return "Concentric";
        case ConstraintType::Angle:         return "Angle";
    }
    return "Constraint";
}

std::string SketchEditOp::description() const {
    if (!m_before || !m_after) return "Sketch edit";

    // Constraint diff first — these read more specifically than the generic
    // geometry-count descriptions below.
    const auto& cBefore = m_before->getConstraints();
    const auto& cAfter  = m_after->getConstraints();
    if (cBefore.size() != cAfter.size()) {
        // Added or removed. Look at the difference set (by id).
        char buf[80];
        if (cAfter.size() > cBefore.size()) {
            // Find the first id present in after but not before.
            for (const auto& c : cAfter) {
                bool wasThere = false;
                for (const auto& b : cBefore) if (b.id == c.id) { wasThere = true; break; }
                if (wasThere) continue;
                const char* name = constraintName(c.type);
                if (c.type == ConstraintType::Distance) {
                    std::snprintf(buf, sizeof(buf), "Add Distance %.2f mm", c.value);
                } else if (c.type == ConstraintType::Radius) {
                    std::snprintf(buf, sizeof(buf), "Add \xC3\x98 %.2f mm", c.value * 2.0);
                } else if (c.type == ConstraintType::Angle) {
                    std::snprintf(buf, sizeof(buf), "Add Angle %.1f\xC2\xB0",
                                  c.value * 180.0 / M_PI);
                } else {
                    std::snprintf(buf, sizeof(buf), "Add %s", name);
                }
                return buf;
            }
            return "Add constraint";
        } else {
            // Removed — name the removed type if we can identify it.
            for (const auto& b : cBefore) {
                bool stillThere = false;
                for (const auto& a : cAfter) if (a.id == b.id) { stillThere = true; break; }
                if (stillThere) continue;
                std::snprintf(buf, sizeof(buf), "Remove %s", constraintName(b.type));
                return buf;
            }
            return "Remove constraint";
        }
    } else {
        // Same count — check for a value edit on the same id.
        for (size_t i = 0; i < cAfter.size(); ++i) {
            // Find matching id in before.
            const Constraint* bMatch = nullptr;
            for (const auto& b : cBefore) if (b.id == cAfter[i].id) { bMatch = &b; break; }
            if (!bMatch) continue;
            if (std::abs(bMatch->value - cAfter[i].value) > 1e-9 ||
                std::abs(bMatch->valueY - cAfter[i].valueY) > 1e-9) {
                char buf[100];
                if (cAfter[i].type == ConstraintType::Angle) {
                    std::snprintf(buf, sizeof(buf), "Edit Angle %.1f\xC2\xB0 \xE2\x86\x92 %.1f\xC2\xB0",
                                  bMatch->value * 180.0 / M_PI,
                                  cAfter[i].value * 180.0 / M_PI);
                } else if (cAfter[i].type == ConstraintType::Radius) {
                    std::snprintf(buf, sizeof(buf), "Edit \xC3\x98 %.2f \xE2\x86\x92 %.2f mm",
                                  bMatch->value * 2.0, cAfter[i].value * 2.0);
                } else if (cAfter[i].type == ConstraintType::Distance) {
                    std::snprintf(buf, sizeof(buf), "Edit Distance %.2f \xE2\x86\x92 %.2f mm",
                                  bMatch->value, cAfter[i].value);
                } else {
                    std::snprintf(buf, sizeof(buf), "Edit %s",
                                  constraintName(cAfter[i].type));
                }
                return buf;
            }
        }
    }

    // No constraint diff — fall back to geometry-count diff (existing behaviour).
    int delta = m_after->elementCount() - m_before->elementCount();
    if (delta > 0) return "Add sketch element";
    if (delta < 0) return "Remove sketch element";
    return "Modify sketch";
}

} // namespace materializr
