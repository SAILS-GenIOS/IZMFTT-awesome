#include "SketchEditOp.h"
#include "SketchSolver.h"
#include <imgui.h>
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

void SketchEditOp::renderProperties() {
    if (!m_after) {
        ImGui::TextDisabled("No snapshot");
        return;
    }
    auto& cs = m_after->getMutableConstraints();
    if (cs.empty()) {
        ImGui::TextDisabled("No constraints in this step");
        return;
    }

    // Edit dimensional values inline. For each change we re-solve `m_after`
    // so dependent geometry catches up — Apply Changes then copies the
    // solved snapshot onto the live sketch via editStep / execute().
    auto resolveAfter = [&]() {
        SketchSolver solver;
        solver.solve(*m_after);
    };

    bool anyDim = false;
    for (size_t i = 0; i < cs.size(); ++i) {
        Constraint& c = cs[i];
        ImGui::PushID(static_cast<int>(i));
        switch (c.type) {
            case ConstraintType::Distance: {
                anyDim = true;
                double v = c.value;
                if (ImGui::InputDouble("Distance (mm)", &v, 0.0, 0.0, "%.3f",
                                       ImGuiInputTextFlags_EnterReturnsTrue)) {
                    c.value = v;
                    resolveAfter();
                }
                break;
            }
            case ConstraintType::Radius: {
                anyDim = true;
                // Stored as radius; show as diameter to match the in-sketch
                // popup ("Ø ..." in descriptions and dimensions).
                double dia = c.value * 2.0;
                if (ImGui::InputDouble("\xC3\x98 (mm)", &dia, 0.0, 0.0, "%.3f",
                                       ImGuiInputTextFlags_EnterReturnsTrue)) {
                    c.value = std::max(dia, 1e-6) * 0.5;
                    resolveAfter();
                }
                break;
            }
            case ConstraintType::Angle: {
                anyDim = true;
                double deg = c.value * 180.0 / M_PI;
                if (ImGui::InputDouble("Angle (\xC2\xB0)", &deg, 0.0, 0.0, "%.2f",
                                       ImGuiInputTextFlags_EnterReturnsTrue)) {
                    c.value = deg * M_PI / 180.0;
                    resolveAfter();
                }
                break;
            }
            default: {
                // Non-dimensional constraints have nothing to tune. Show the
                // name as a read-only row so the user can confirm what's in
                // the step, then move on.
                const char* name = constraintName(c.type);
                ImGui::TextDisabled("• %s", name);
                break;
            }
        }
        ImGui::PopID();
    }

    if (!anyDim) {
        ImGui::TextWrapped("This step contains only non-dimensional "
                           "constraints — there are no values to edit.");
    } else {
        ImGui::TextDisabled("Press Enter to commit a value, then Apply Changes.");
    }
}

} // namespace materializr
